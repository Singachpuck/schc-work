# SCHC Work

## Description

The project indents to provide comprehensive usecases for SCHC Full SDK provided by SCHC Lab:

https://gitlab.com/lab-schc/sdk/full-sdk-delivery

## Applications

1. Demo - dummy application that is used to test the build.
2. LPWAN Model - Application that models LPWAN communication scenario:
   * Device - generates and sends SCHC-compressed CoAP packet over IPv4 tunnel.
   * SCHC Core - receives and decompresses SCHC packet and forwards it to the Application.
   * Application - receives and processes IPv6/UDP/CoAP packet as a normal packet.

## CoAP/OSCORE \w SCHC Setup

1. There are two linux processes (RTOS uses tasks):
2. First, SCHC adaptation layer process. It opens a linux tunnel which intercepts any ip traffic on the linux machine.
3. Whenever any other application sends a packet to the kernel, the kernel will route the packets to the tunnel.
4. Then, the SCHC AL reads the packet from the tunnel and compresses IPv6/UDP/CoAP/[OSCORE].