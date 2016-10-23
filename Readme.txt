Author : Surjith Bhagavath Singh
File details	: Read me for Web Server (Programming Assignment 2 - Network Systems)

There are three main files for this programming assignment
	1.server.c
	2.Root_webpage(Directory)
	3.ws.conf
	4.Makefile

server.c
	This file serves as the server for this assignment. It takes it's arguments from ws.conf file. If ws.conf is not in the directory of server.c, web server will not start (Throwsan error)
	eg - ./server (Make sure the value of port is greater than 1024)

	Listen of upto 50 clients, Multithreading has been performed to serve all the clients.
	Keepalive time - This value is taken from ws.conf and it sets the value if there is a "Connection: keep-alive" Header from the client.
	It can gracefully exit on Ctrl+C
	Pipeline has been implemented using pthreads.
	POST is implemented for a specific webpage(abc.html)
	Error Handling for 404,400,500,501 Codes has been implemented

ws.conf
	This file handles all the configuration for the web server. Root Directory, files to be supported , KeepAlivetime. These parameters can be configured using this file

Root_webpage
	This directory includes the sample webpage with image/gif/text/PNG/html/css files

Instructions to use the code:
	Make Sure ws.conf and server.c and Makefile in the same directory. Make sure to update the root value in ws.conf. 
	1. "make all" from terminal
	2. ./server
	3. Go to a web browser and type localhost:port/ and press enter
