# IoT Smart Gateway (IoTSGW) [![published](https://static.production.devnetcloud.com/codeexchange/assets/images/devnet-published.svg)](https://developer.cisco.com/codeexchange/github/repo/SoftServeInc/IoT-Smart-Gateway)
Smart gateway application for observing IoT devices & other services. IoTSGW is an edge data collector with the Web UI, where user can create, delete and observe IoT devices through different protocols.<br>
The straight forward API, ability to run on Docker and IOx compatible devices (such as Cisco IR-809), outstanding speed as well as low memory requirements, simple UI, all these are the benefits of an IoTSGW.<br>
## Solution diagram
![cli snapshot](/img/schema.png)
## Deployment
### Docker scenario
To download this project, use whatever is more convenient for you, e.g. git. After download, you could deploy it and assign corresponding port.
```
$ git clone https://github.com/OpenSourceASAContributors/iotsmartgateway.git
$ cd iotsmartgateway
$ docker-compose run make # creates build folder and populates it
$ docker-compose run -p 80:80 emulate
```
So, after passing to the browser and typing "localhost" in the address bar, you will see the home page. At the same time, in the same terminal, where the docker iotsgw container had been started, you will see the CLI logs accounting that HTTP & CoAP servers are in the UP state.
For now, CLI could be used for debugging purposes albeit its only legacy purpose is an obvious "exit".
### IOx container scenario
The procedure is straight forward, if you want to alter IOx any of an iotsgwapp' metadata, just open and modify the package.yaml. Note. In the IOx mode, the CLI will be not available, although Web UI will remain.
```
$ git clone https://github.com/OpenSourceASAContributors/iotsmartgateway.git
$ cd iotsmartgateway
$ docker-compose run make
$ docker rmi iotsgwapp # to clear the previous image (in case you have one)
$ docker build --tag iotsgwapp --output type=tar.dest=rootfs.tar .
$ ioxclient package . # download ioxclient from https://software.cisco.com/download/home/286306005/type/286306762/release/1.10.0
$ mv package.tar iotsgwapp.tar # grab your IOx package
```
## API reference
### POST <app-hostname>/add-new-observable
Is used to add new entity for observation.<br>
Sample request body:
```
{
    "device-location": "https://samples.openweathermap.org/data/2.5/weather?q=London,uk&appid=439d4b804bc8187953eb36d2a8c26a02",
    "param-location": "main.temp",
    "device-name": "garden-detector",
    "param-name": "temperature",
    "interval": 5
}
```
- interval: period in seconds between measurements
- param-name: name of the parameter (convenience purposes)
- device-name: name of the iot device (convenience purposes)
- param-location: '.' separated path to parameter in the json response
- device-location: the url that allows to get a response with data<br>
Response body:
```
{ "result": "success", "handle": 1037 }
```
- result: result of an operation
- handle: the number that is used to access the specific observable
### POST <app-hostname>/update-observable/\<handle>
Is used to update an existing observable entity.<br>
Sample request body:
```
{
    "param-location": "main.temp",
    "device-name": "garden-detector",
    "param-name": "temperature",
}
```
- param-name: new name of the parameter (convenience purposes)
- device-name: new name of the iot device (convenience purposes)
- param-location: new '.' separated path to parameter in the json response<br>
Sample response body:
```
{ "result": "success" }
```
- result: result of an operation
### POST <app-hostname>/remove-observable/\<handle>
No request body is needed. Is used to delete an existing observable entity.<br>
Sample response body:
```
{ "result": "success" }
```
- result: result of an operation
### POST <app-hostname>/observe-once
Sample request body:
```
{ "device-location": "https://samples.openweathermap.org/data/2.5/weather?q=London,uk&appid=439d4b804bc8187953eb36d2a8c26a02" }
```
Is used to check the current json response by the given url:
- device-location: the url that allows to get a response with data
Sample response body if the url response is json:
```
{
    "main": {
        "temp": 20,
        "pressure": 1000,
        "humidity": 80
    },
    "service": "up"
}
```
Note. if the given url is either not responding or its response is not json, the response body will be an empty json ("{}").
## GET <app-hostname>/get-uptime
Is used to obtain the longevity of an application being active.<br>
Response body:
```
{ "uptime": <uptime> }
```
- uptime: duration in seconds for which the application is active
### GET <app-hostname>/get-iotdata/\<handle>
Is used to obtain collected data for the specified observable by its handle.<br>
Sample response body:
```
{
    "name": "garden-detector",
    "param": "humidity",
    "handle": "1932",
    "values": [
        {
            "date": "2020-11-10T06:40:23Z",
            "value": "81"
        },
        {
            "date": "2020-11-10T06:40:34Z",
            "value": "81"
        },
        {
            "date": "2020-11-10T06:40:45Z",
            "value": "81"
        },
        {
            "date": "2020-11-10T06:40:55Z",
            "value": "81"
        },
        {
            "date": "2020-11-10T06:41:06Z",
            "value": "81"
        },
        {
            "date": "2020-11-10T06:41:16Z",
            "value": "81"
        }
    ]
}
```
- name: the specified device name
- param: the specified parameter name
- handle: the handle that is used to access observable
- values: the circular 50 elem array of data, collected with the specified url and path
- values[i].date: the time point when the chosen value has been collected
- values[i].value: the collected value, type is double
### GET <app-hostname>/get-iotdata
Is used to obtain all collected data. Returns an array of observables.<br>
Sample response body:
```
{
    "observables": [
        {
            "name": "garden-detector",
            "param": "humidity",
            "handle": "1932",
            "values": [
                {
                    "date": "2020-11-10T06:40:23Z",
                    "value": "81"
                },
                {
                    "date": "2020-11-10T06:40:34Z",
                    "value": "81"
                },
                {
                    "date": "2020-11-10T06:40:45Z",
                    "value": "81"
                }
            ]
        },
        {
            "name": "garden-detector",
            "param": "temperature",
            "handle": "1037",
            "values": [
                {
                    "date": "2020-11-10T06:39:33Z",
                    "value": "280.31999999999999"
                },
                {
                    "date": "2020-11-10T06:39:39Z",
                    "value": "280.31999999999999"
                },
                {
                    "date": "2020-11-10T06:39:44Z",
                    "value": "280.31999999999999"
                },
                {
                    "date": "2020-11-10T06:39:52Z",
                    "value": "280.31999999999999"
                },
                {
                    "date": "2020-11-10T06:39:57Z",
                    "value": "280.31999999999999"
                },
                {
                    "date": "2020-11-10T06:40:02Z",
                    "value": "280.31999999999999"
                },
                {
                    "date": "2020-11-10T06:40:07Z",
                    "value": "280.31999999999999"
                },
                {
                    "date": "2020-11-10T06:40:13Z",
                    "value": "280.31999999999999"
                },
                {
                    "date": "2020-11-10T06:40:18Z",
                    "value": "280.31999999999999"
                },
                {
                    "date": "2020-11-10T06:40:23Z",
                    "value": "280.31999999999999"
                },
                {
                    "date": "2020-11-10T06:40:29Z",
                    "value": "280.31999999999999"
                },
                {
                    "date": "2020-11-10T06:40:34Z",
                    "value": "280.31999999999999"
                },
                {
                    "date": "2020-11-10T06:40:39Z",
                    "value": "280.31999999999999"
                },
                {
                    "date": "2020-11-10T06:40:44Z",
                    "value": "280.31999999999999"
                }
            ]
        }
    ]
}
```
- observables[i].name: the specified device name
- observables[i].param: the specified parameter name
- observables[i].handle: the handle that is used to access observable
- observables[i].values: the circular 50 elem array of data, collected with the specified url and path
- observables[i].values[j].date: the time point when the chosen value has been collected
- observables[i].values[j].value: the collected value, type is double
### Response errors
Is used to cover parsing and access failures.<br>
- 400 "Bad format, json is expected": if request body is not json
- 400 "Bad format, members are missing": if request body is missing some parameters
- 400 "Bad interval in seconds, use int": if interval value in add-new-observable is not an integer
- 404 "Bad handle, no observable found": if the handle of an observable is invalid
- 400 "Bad param location, use: path.to.param": if the param-location in add-new-observable has invalid format
- 400 "Bad URL, use: proto://address[:port]/[path]": if the device-location in add-new-observable or in observe-once has invalid format
## Examples
### webui
![My devices](/img/devices.png)
![Dashboard](/img/dashboards.png)
![Add new device](/img/addnewdevice.png)
### cli
![cli snapshot](/img/cli.png)
## Authors
- Ihor Berezhnyi (berezhnyj95@gmail.com)
- Valentyn Faychuk (faitchouk.valentyn@gmail.com)
- Many others
## Contribution
The IoT Smart Gateway is open for any sort of contribution or bug reporting.<br>
Please, contact either Valentyn Faychuk or Ihor Berezhnyi for details.
## Thanks
This project is maintained by Valentyn Faychuk and Ihor Berezhnyi and a set of other contributors.<br>
<br>
Originally the application was developed to observe SoftServe internal resource but recenntly we have decided to push it to opensource.
