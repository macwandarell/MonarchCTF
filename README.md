# MonarchCTF

## Description
A multi-user shared CTF-room environment mainly focused on forensics challenges (for now :D) which runs on a server.

## Dependencies
- openssl
- POSIX threads
- lifuse3-dev

## Instructions
- For Admin/Server
  - Do ' echo "user_allow_other " >> /etc/fuse.conf ' (first time setup only)
  - First run the server using "./build/main 0"
  - You can change the admin password from the server.c file (highly recommended to do so)
  - To add problems, first make the problem and its description inside the questions/ directory
  - "bash" is a mandatory thing which you should add using the admin interface
  - For other commands, please make sure they already exist in your system
  - In case of any major issues/errors, stop the server, and remove/edit the data/ files if necessary
  - The user working directories and the jail directory will be made in the "/home/ctf" directory
  - During distribution, just provide the client with the "main.c","custom_error.c/h","client.c/h" files

- For clients
  - Compile and run using "./main 1"
  - Just login and enjoy playing!

## Features
- User doesn't need to have the tools installed on their side, the shell runs on the server
- The shell is a PTY based interactive shell
- Users can make teams and collaborate
- Sandboxing has been done using chroot
- Users are restricted to a "jail" and can only run binaries specified by the admin
- Users can submit answers and check points and contribute to their teams
- Using FUSE for concurrency control in shared rooms

## Future additions
~~Currently, room-level isolation is implemented, however inside a room, users still need to collaborate, we want to add isolation features at user level in the future (basically what kernel does, since pty by itself cant do that)~~
We have implemented concurrency control using FUSE, we need to enhance this feature itself by making it more rigid and by adding more syscalls.

Furthermore, we want to support more types of challenges, even right now, admin/server can basically add the required binaries to solve cryptography and binary exploitation. But, we want to add actual interface which can help users to reverse engineering, web-exploitation and osint as well using server-resources itself, which will also be required to run on low latency.

Make the "jail" enviroment very secure, right now, it is minimal, and as we will begin to add more features to it, and test it more, we can detect bugs and prevent exploitation attempts (especially RCE). Contributions are welcome here


## Acknowledgements
Some of the demo challenges which are used are sourced from https://play.picoctf.org/ 

All rights to those challenges belong to their respective creators. A huge respect to their team.

libfuse (https://github.com/libfuse/libfuse)

## Inspiration
The idea of the platform itself is inspired by my interest in CTF and the idea of creating an environment in which the user can solve challenges without having to install tools on their system. The name "MonarchCTF" is inspired by ML.Iseria, also known as "Monarch of the sword" from the game Epic7, which basically reflects dominance.
