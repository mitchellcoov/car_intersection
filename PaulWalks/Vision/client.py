from socket import *

def stopClient(area):
    serverName = '127.0.0.1'
    serverPort = 13000
    clientSocket = socket(AF_INET, SOCK_STREAM)
    clientSocket.connect((serverName, serverPort))
    clientSocket.send(area)
    clientSocket.recv(1024)
    clientSocket.close()
