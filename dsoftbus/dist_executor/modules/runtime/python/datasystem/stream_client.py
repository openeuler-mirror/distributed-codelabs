# Copyright (c) 2022 Huawei Technologies Co., Ltd.
#
# This software is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
# http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

"""
Stream cache client python interface.
"""

from enum import Enum
import datasystem.libds_client_py as ds


class SubconfigType(Enum):
    """The type of stream"""
    STREAM = 0
    ROUND_ROBIN = 1
    KEY_PARTITIONS = 2


class StreamClient:
    """the client of stream"""

    def __init__(self, host: str, port: int, client_public_key: str = "",
                 client_private_key: str = "", server_public_key: str = ""):
        """ Constructor of the StreamClient class

        Args:
            host(str): The worker address host.
            port(str): The worker address port.
            client_public_key(str): The client's public key, for curve authentication.
            client_private_key(str): The client's private key, for curve authentication.
            server_public_key(str): The worker server's public key, for curve authentication.
        """
        self._client = ds.StreamClient(host, port, client_public_key, client_private_key, server_public_key)

    def init(self):
        """ Init a stream client to connect to a worker.

        Raises:
            RuntimeError: Raise a runtime error if the client fails to connect to the worker.
        """
        init_status = self._client.init()
        if init_status.is_error():
            raise RuntimeError(init_status.to_string())

    def create_producer(self, stream_name, delay_flush_time_ms=5, page_size_byte=1024 * 1024,
                        max_stream_size_byte=1024 * 1024 * 1024):
        """ Create one Producer to send element.

        Args:
            stream_name: The name of the stream.
            delay_flush_time_ms: The time used in automatic flush after send and default is 5ms.
            page_size_byte: The size used in allocate page and default is 1MB.
                must be a multiple of 4KB.
            max_stream_size_byte: The max stream size in worker and default is 1GB.
                must between greater then 64KB and less than the shared memory size.
        Return:
            outProducer: The output Producer that user can use it to send element.

        Raise:
            TypeError: Raise a type error if the input parameter is invalid.
            RutimeError: Raise a runtime error if creating a producer fails.
        """
        if not isinstance(stream_name, str):
            raise TypeError("The input of stream_name should be string.")
        if not isinstance(delay_flush_time_ms, int):
            raise TypeError("The input of delay_flush_time_ms should be int.")
        if not isinstance(page_size_byte, int):
            raise TypeError("The input of page_size_byte should be int.")
        status, out_producer = self._client.CreateProducer(
            stream_name, delay_flush_time_ms, page_size_byte, max_stream_size_byte)
        if status.is_error():
            raise RuntimeError(status.to_string())
        return Producer(out_producer)

    def subscribe(self, stream_name, sub_name, subscription_type):
        """ Subscribe a new consumer onto master request

        Args:
            stream_name: The name of the stream.
            sub_name: The name of subscription
            subscription_type: The type of subscription.

        Return:
            status: Status of the call.

        Raise:
            TypeError: Raise a type error if the input parameter is invalid.
            RuntimeError: Raise a runtime error if subscribing a new consumer fails.
        """
        if not isinstance(stream_name, str):
            raise TypeError("The input of stream_name should be string.")
        if not isinstance(subscription_type, int):
            raise TypeError("The input of type should be int.")
        status, out_consumer = self._client.Subscribe(stream_name, sub_name, subscription_type)
        if status.is_error():
            raise RuntimeError(status.to_string())
        return Consumer(out_consumer)

    def delete_stream(self, stream_name):
        """ Delete one stream

        Args:
            stream_name: The name of the stream.

        Raise:
            TypeError: Raise a type error if the input parameter is invalid.
            RutimeError: Raise a runtime error if deleting one stream fails.
        """
        if not isinstance(stream_name, str):
            raise TypeError("The input of stream_name should be string.")
        status = self._client.DeleteStream(stream_name)
        if status.is_error():
            raise RuntimeError(status.to_string())

    def query_global_producer_num(self, stream_name):
        """ Query number of producer in global worker node

        Args:
            stream_name: The name of the target stream.

        Returns:
            global_producer_num: Query result.

        Raise:
            TypeError: Raise a type error if the input parameter is invalid.
            RutimeError: Raise a runtime error if querying global producer number fails.
        """
        if not isinstance(stream_name, str):
            raise TypeError("The input of stream_name should be string.")
        status, global_producer_num = self._client.QueryGlobalProducersNum(stream_name)
        if status.is_error():
            raise RuntimeError(status.to_string())
        return global_producer_num

    def query_global_consumer_num(self, stream_name):
        """ Query number of consumer in global worker node

        Args:
            stream_name: The name of the target stream.

        Returns:
            global_consumer_num: Query result.

        Raise:
            TypeError: Raise a type error if the input parameter is invalid.
            RutimeError: Raise a runtime error if querying global consumer number fails.
        """
        if not isinstance(stream_name, str):
            raise TypeError("The input of stream_name should be string.")
        status, global_consumer_num = self._client.QueryGlobalConsumersNum(stream_name)
        if status.is_error():
            raise RuntimeError(status.to_string())
        return global_consumer_num


class Producer:
    """the producer of stream in client"""

    def __init__(self, producer):
        if not isinstance(producer, ds.Producer):
            raise TypeError("The input of parament should be Producer.")
        self._producer = producer

    def send(self, element_bytes):
        """ Produce send one element of the stream each time

        Args:
            element: The element that to be written.

        Raise:
            TypeError: Raise a type error if the input parameter is invalid.
            RutimeError: Raise a runtime error if sending one element fails.
        """
        if not isinstance(element_bytes, memoryview) and not isinstance(element_bytes, bytes) and not isinstance(
                element_bytes, bytearray):
            raise TypeError("The input of parament should be memoryview or bytes or bytearray.")
        status = self._producer.Send(element_bytes)
        if status.is_error():
            raise RuntimeError(status.to_string())

    def flush(self):
        """ Produce send one element of the stream each time

        Raise:
            RuntimeError: Raise a runtime error if sending one element fails.
        """
        status = self._producer.Flush()
        if status.is_error():
            raise RuntimeError(status.to_string())

    def close(self):
        """ Close a producer, register a publisher to a stream.

        Raise:
            RuntimeError: Raise a runtime error if closing a producer fails.
        """
        status = self._producer.Close()
        if status.is_error():
            raise RuntimeError(status.to_string())


class Consumer:
    """the consumer of stream in client"""

    def __init__(self, consumer):
        if not isinstance(consumer, ds.Consumer):
            raise TypeError("The input of parament should be Consumer.")
        self._consumer = consumer

    def receive(self, except_num, timeout_ms):
        """ Receive elements meta, where worker handles big and small element lookup and parsing

        Args:
            except_num: The number of elements to be read.
            timeout_ms: The timeout millisecond of elements to be Receive.

        Return:
            values: element has been received

        Raise:
            TypeError: Raise a type error if the input parameter is invalid.
            RutimeError: Raise a runtime error if receiving elements meta falis.
        """
        if not isinstance(except_num, int):
            raise TypeError("The input of except_num should be int.")
        if not isinstance(timeout_ms, int):
            raise TypeError("The input of timeout_ms should be int.")
        status, element = self._consumer.Receive(except_num, timeout_ms)
        if status.is_error():
            raise RuntimeError(status.to_string())
        return element

    def ack(self, element_id):
        """ Acknowledge elements that had been read by this consumer.

        Args:
            element_id: The element id that to be acknowledged.

        Raise:
            TypeError: Raise a type error if the input parameter is invalid.
            RutimeError: Raise a runtime error if acknowledging elements falis.
        """
        if not isinstance(element_id, int):
            raise TypeError("The input of element_id should be int.")
        status = self._consumer.Ack(element_id)
        if status.is_error():
            raise RuntimeError(status.to_string())

    def close(self):
        """ Close the consumer, after close it will not allow Receive and Ack Elements.

        Raise:
            RuntimeError: Raise a runtime error if closing the consumer falis.
        """
        status = self._consumer.Close()
        if status.is_error():
            raise RuntimeError(status.to_string())
