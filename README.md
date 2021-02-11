# Record Manager

[![Build Status](https://travis-ci.com/johnpatek/record-manager.svg?branch=master)](https://travis-ci.com/johnpatek/record-manager)

Build:
```shell
python3 cmake.py --external
python3 cmake.py
```

Start the server application:
```shell
mkdir /home/ubuntu/records
server 12345 /home/ubuntu/records
```

Start the client application:
```shell
client 127.0.0.1 12345
```
