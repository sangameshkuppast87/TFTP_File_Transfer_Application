# TFTP File Transfer Application

## Overview

This project implements a basic **TFTP (Trivial File Transfer Protocol)** application using **C and UDP sockets**.
The system consists of a **client** and **server** that communicate using the TFTP protocol to transfer files over a network.

The project demonstrates concepts of:

* Socket programming
* UDP communication
* Client–server architecture
* File transfer protocols

---

## Features

* Client–Server file transfer using UDP
* Support for basic TFTP operations:

  * `connect` – Connect to TFTP server
  * `get` – Download file from server
  * `put` – Upload file to server
  * `mode` – Change transfer mode
  * `exit` – Terminate client
* Packet based data transfer
* Command-line interface
* Makefile for easy compilation

---

## Project Structure

```
TFTP/
│
├── client/
│   ├── tftp_client.c
│   ├── tftp_client.h
│   ├── tftp.c
│   ├── tftp.h
│   └── Makefile
│
├── server/
│   ├── tftp_server.c
│   ├── tftp.c
│   ├── tftp.h
│   └── Makefile
│
└── README.md
```

---

## Compilation

### Compile Client

```
cd client
make
```

### Compile Server

```
cd server
make
```

---

## Running the Program

### Start Server

```
./server
```

### Start Client

```
./client
```

The client will display a menu:

```
1. connect
2. get
3. put
4. mode
5. exit
```

---

## Technologies Used

* C Programming
* UDP Socket Programming
* Linux System Calls
* Makefiles

---

## Learning Outcomes

This project helped in understanding:

* UDP based communication
* Implementation of TFTP protocol
* Packet handling and acknowledgements
* Client–server network programming in C

---

## Author

Sangamesh Irappa Kuppast
