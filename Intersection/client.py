from socket import *

def stopClient(x,y,w,h):
    serverName = '127.0.0.1'
    serverPort = 13000
    clientSocket = socket(AF_INET, SOCK_STREAM)
    clientSocket.connect((serverName, serverPort))
    clientSocket.send(str(x) + "," + str(y) + "," + str(w) + "," + str(h) + "\0")
    clientSocket.recv(1024)
    clientSocket.close()
