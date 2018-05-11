import threading

import time

from thrift.transport.TTransport import TTransportException

from mmux.directory.directory_client import DirectoryClient
from mmux.subscription.subscriber import SubscriptionClient, Mailbox
from mmux.lease.lease_client import LeaseClient
from mmux.kv.kv_client import KVClient
import logging

logging.basicConfig(level=logging.WARN,
                    format="%(asctime)s %(levelname)s %(name)s %(message)s",
                    datefmt="%Y-%m-%d %X")

logging.getLogger('thrift').setLevel(logging.FATAL)


class RemoveMode:
    def __init__(self):
        pass

    delete = 0
    flush = 1


def now_ms():
    return int(round(time.time() * 1000))


class LeaseRenewalWorker(threading.Thread):
    def __init__(self, ls, to_renew):
        super(LeaseRenewalWorker, self).__init__()
        self.ls = ls
        self.to_renew = to_renew
        self.renewal_duration_s = float(self.ls.renew_leases([]).lease_period_ms / 1000.0)
        self._stop_event = threading.Event()

    def run(self):
        while not self.stopped():
            # TODO: We can use semaphores to block until we have leases to renew instead
            try:
                s = now_ms()
                # Only update lease if there is something to update
                if self.to_renew:
                    ack = self.ls.renew_leases(self.to_renew)
                    n_renew = len(self.to_renew)
                    self.renewal_duration_s = float(ack.lease_period_ms / 1000.0)
                    if ack.renewed != n_renew:
                        logging.warning('Asked to renew %d leases, server renewed %d leases' % (n_renew, ack.renewed))
                elapsed_s = float(now_ms() - s) / 1000.0
                sleep_time = self.renewal_duration_s - elapsed_s
                if sleep_time > 0.0:
                    time.sleep(sleep_time)
            except TTransportException as e:
                logging.warning("Connection error: %s" % repr(e))
                break

    def stop(self):
        self._stop_event.set()

    def stopped(self):
        return self._stop_event.is_set()


class ChainFailureCallback:
    def __init__(self, fs):
        self.fs = fs

    def __call__(self, path, chain):
        self.fs.resolve_failures(path, chain)


class MMuxClient:
    def __init__(self, host="127.0.0.1", directory_service_port=9090, lease_port=9091):
        self.directory_host = host
        self.directory_port = directory_service_port
        self.fs = DirectoryClient(host, directory_service_port)
        self.chain_failure_cb = ChainFailureCallback(self.fs)
        self.notifs = {}
        self.to_renew = []
        self.ls = LeaseClient(host, lease_port)
        self.lease_worker = LeaseRenewalWorker(self.ls, self.to_renew)
        self.lease_worker.daemon = True
        self.lease_worker.start()

    def __del__(self):
        self.disconnect()

    def disconnect(self):
        self.lease_worker.stop()
        self.fs.close()
        self.ls.close()

    def keep_alive(self, path):
        if path not in self.to_renew:
            self.to_renew.append(path)

    def create(self, path, persistent_store_prefix, num_blocks=1, chain_length=1):
        s = self.fs.create(path, persistent_store_prefix, num_blocks, chain_length)
        self.keep_alive(path)
        return KVClient(self.fs, path, s, self.chain_failure_cb)

    def open(self, path):
        s = self.fs.open(path)
        self.keep_alive(path)
        return KVClient(self.fs, path, s, self.chain_failure_cb)

    def open_or_create(self, path, persistent_store_prefix, num_blocks=1, chain_length=1):
        s = self.fs.open_or_create(path, persistent_store_prefix, num_blocks, chain_length)
        self.keep_alive(path)
        return KVClient(self.fs, path, s, self.chain_failure_cb)

    def close(self, path):
        if path in self.to_renew:
            self.to_renew.remove(path)

    def remove(self, path, mode):
        self.close(path)
        if mode == RemoveMode.delete:
            self.fs.remove_all(path)
        elif mode == RemoveMode.flush:
            self.fs.flush(path)

    def open_listener(self, path, callback=Mailbox()):
        s = self.fs.dstatus(path)
        if path not in self.to_renew:
            self.to_renew.append(path)
        return SubscriptionClient(s, callback)