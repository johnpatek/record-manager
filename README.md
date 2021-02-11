# Record Manager

[![Build Status](https://travis-ci.com/johnpatek/record-manager.svg?branch=master)](https://travis-ci.com/johnpatek/record-manager)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/johnpatek/record-manager.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/johnpatek/record-manager/alerts/)

Build:
```shell
python3 cmake.py --common
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
