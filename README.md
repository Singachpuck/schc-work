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