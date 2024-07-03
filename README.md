# Module of secret storage
## Some words about module
The secret storage module is a moduie for working with secrets/ In the context of kernel development secret is pair of key:value.
This module consists of three functions: getting, setting and deleting secrets.
For secret's creating the hash function is used. So hash creates pair of key and its value.
More information about functions you will find right in code of ```ssmod.c``` file.
## Building project
To build the kernel module you need to write next command ```make``` in the directory with files.
## Some technic information
This code was made on Ubuntu 22.04, with kernel 6.5.0-41-generic.
