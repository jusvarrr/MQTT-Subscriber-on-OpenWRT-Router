# MQTT-Subscriber-on-OpenWRT-Router

This program implements an MQTT subscriber and publisher system on a OpenWRT router. It includes features like subscription to multiple MQTT topics, user authentication, data encryption using TLS, event-based notifications to email, and logging. The user can configure multiple topics and receive notifications when specific messages containing matching topics, paramters and condition is received from these topics. Also it can view events logs on specific by running the script filtering by topic or date. 

## Functional Requirements:
- Multiple Topic Subscriptions
- Topic Data Viewing in a database
- MQTT User Authentication can be used
- TLS Encryption can be used
- Event Rules
- Logging
- Background Operation

## Non-Functional Requirements:
- Written in C.
- **Libraries**:
  - Mosquitto: Provides MQTT functionality for both the publisher and subscriber.
  - libcurl: Used to send email notifications when an event rule is triggered.
  
## Getting Started

These instructions will help you set up and run the program on your router.


### Installation
#### Prerequisites

Ensure the following software and libraries are installed on your router:
- **Mosquitto**: For MQTT functionality.
- **libcurl**: For sending emails.
- **OpenSSL**: For TLS encryption.
- **lsqlite3**: For saving topics data and getting data saved with queries.
- **argp_standalone**: For reading configurations from configuration file of the package.
- **cJSON**: for formatting and parsing MQTT messages data.

### 1. Clone the repository:
   ```bash
   git clone https://github.com/your-username/MQTT-Subscriber-on-OpenWRT-Router.git
   cd MQTT-Subscriber-on-OpenWRT-Router
  ```
### 2. Compile packages mqttsub in OpenWRT host machine environment with:
  ```bash
    make package/mqttsub/compile
  ```
### 3. Deploy the installation file from build directory on OpenWRT environment on Your host machine to the router using ssh:
   
bash
   scp bin/packages/*/*.ipk device@<device_ip>:/tmp/


### 4. On the device run:
   
bash
   opkg install /tmp/*.ipk
