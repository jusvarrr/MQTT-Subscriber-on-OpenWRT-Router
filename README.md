# MQTT-Subscriber-on-OpenWRT-Router

This package "mqttsub" implements an MQTT subscriber and publisher system on a OpenWRT router. It includes features like subscription to multiple MQTT topics, user authentication, data encryption using TLS, event-based notifications to email, and logging. The user can configure multiple topics and receive notifications when specific messages containing matching topics, paramters and condition are received by the broker running on the same router. Also event logs can be viewed by running the script and filtering by topic or date if needed.

## Features:
- Multiple Topic Subscriptions
- Topic Data Viewing in a database with filters
- MQTT User Authentication can be used
- TLS Encryption can be used
- Event Rules
- Logging
- Background Service running as daemon

### Installation
#### Prerequisites

Ensure the following software and libraries are installed on your router:
- **Mosquitto**: For MQTT functionality.
- **libcurl**: For sending emails.
- **OpenSSL**: For TLS encryption.
- **lsqlite3**: For saving topics data and getting data saved with queries.
- **argp_standalone**: For reading configurations from configuration file of the package.
- **cJSON**: for formatting and parsing MQTT messages data.
- Your router should already have UCI configuration email addresses looking like:
  ```config
  config email '1'
    option smtp_ip 'smtp.example.com'
    option username 'your_username'
    option password 'your_password'
    option senderemail 'youremail@example.com'
    option smtp_port '587'
``
- Events are listed in the configuration file: ```/etc/config/subscriber```

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

### 5. Having the package installed and service running, You can read the database to see logs from topics
- Filter by date (can provide start and end date, only start date or only end date):
  ```bash
   /usr/bin/readlog.lua --date_from "start_date" --date_to "end_date"
  ```
- Filter by topics:
  ```bash
  /usr/bin/readlog.lua --topics "topic_name"
  ```
