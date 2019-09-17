#! /usr/bin/python3

import liblo
from liblo import *
import queue, sys, time

responses = queue.Queue()

class OscServer(ServerThread):
    def __init__(self):
        ServerThread.__init__(self, 9458)
        
    @make_method('/silica/display/startup_system_process', 'si')
    def osc_address_handler(self, path, args):
        s = args
        responses.put(s)
        print("received '/silica/display/startup_system_process'")
        
    @make_method(None, None)
    def fallback(self, path, args):
        print("fallback, received '{}'".format(path))

def test_silica_display(dest):    
    liblo.send(dest, "/silica/display/startup_system_process", "linux 4.14.83-gentoo", 0)
    response = responses.get()
    print('response', response)
    
    liblo.send(dest, "/silica/display/startup_system_process", "linux 4.14.83-gentoo", 1)
    response = responses.get()
    print('response', response)

    liblo.send(dest, "/silica/display/startup_system_process", "jackd", 0)
    response = responses.get()
    print('response', response)
    
    liblo.send(dest, "/silica/display/startup_system_process", "jackd", 1)
    response = responses.get()
    print('response', response)
    
if __name__ == '__main__':
    #outgoing connection
    dest = liblo.Address(9456)

    #incoming server
    server = OscServer()

    server.start()

    test_silica_display(dest)

    input("press enter to quit...\n")
