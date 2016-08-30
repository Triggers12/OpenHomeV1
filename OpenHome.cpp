/* OpenHome Firmware
 * Copyright (C) 2015 by Charles Remeikas
 *
 * OpenHome library
 * Feb 2015 @ OpenHome.com
 *
 * This file is part of the OpenHome library
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <netdb.h>
#include "OpenHome.h"
#include "gpio.h"

/** Declare static data members */
NVConData OpenHome::nvdata;
ConStatus OpenHome::status;
ConStatus OpenHome::old_status;
byte OpenHome::hw_type;

byte OpenHome::nboards;
byte OpenHome::nstations;
byte OpenHome::station_bits[MAX_EXT_BOARDS+1];

ulong OpenHome::sensor_lasttime;
ulong OpenHome::flowcount_log_start;
ulong OpenHome::flowcount_rt;
ulong OpenHome::flowcount_time_ms;
ulong OpenHome::raindelay_start_time;
byte OpenHome::button_timeout;
ulong OpenHome::checkwt_lasttime;
ulong OpenHome::checkwt_success_lasttime;

char tmp_buffer[TMP_BUFFER_SIZE+1];       // scratch buffer

const char wtopts_filename[] = WEATHER_OPTS_FILENAME;
const char stns_filename[]   = STATION_ATTR_FILENAME;

#if defined(OSPI)
  byte OpenHome::pin_sr_data = PIN_SR_DATA;
#endif

/** Option json names (stored in progmem) */
// IMPORTANT: each json name is strictly 5 characters
// with 0 fillings if less
#define OP_JSON_NAME_STEPSIZE 5
const char op_json_names[] =
    "fwv\0\0"
    "tz\0\0\0"
    "ntp\0\0"
    "dhcp\0"
    "ip1\0\0"
    "ip2\0\0"
    "ip3\0\0"
    "ip4\0\0"
    "gw1\0\0"
    "gw2\0\0"
    "gw3\0\0"
    "gw4\0\0"
    "hp0\0\0"
    "hp1\0\0"
    "hwv\0\0"
    "ext\0\0"
    "seq\0\0"
    "sdt\0\0"
    "mas\0\0"
    "mton\0"
    "mtof\0"
    "urs\0\0"
    "rso\0\0"
    "wl\0\0\0"
    "den\0\0"
    "ipas\0"
    "devid"
    "con\0\0"
    "lit\0\0"
    "dim\0\0"
    "bst\0\0"
    "uwt\0\0"
    "ntp1\0"
    "ntp2\0"
    "ntp3\0"
    "ntp4\0"
    "lg\0\0\0"
    "mas2\0"
    "mton2"
    "mtof2"
    "fwm\0\0"
    "fpr0\0"
    "fpr1\0"
    "re\0\0\0"
    "reset";

/** Option promopts (stored in progmem, for LCD display) */
// Each string is strictly 16 characters
// with SPACE fillings if les
char op_promopts[] =
    "Firmware version"
    "Time zone (GMT):"
    "Enable NTP sync?"
    "Enable DHCP?    "
    "Static.ip1:     "
    "Static.ip2:     "
    "Static.ip3:     "
    "Static.ip4:     "
    "Gateway.ip1:    "
    "Gateway.ip2:    "
    "Gateway.ip3:    "
    "Gateway.ip4:    "
    "HTTP Port:      "
    "----------------"
    "Hardware version"
    "# of exp. board:"
    "----------------"
    "Stn. delay (sec)"
    "Master 1 (Mas1):"
    "Mas1  on adjust:"
    "Mas1 off adjust:"
    "Sensor type:    "
    "Normally open?  "
    "Watering level: "
    "Device enabled? "
    "Ignore password?"
    "Device ID:      "
    "LCD contrast:   "
    "LCD brightness: "
    "LCD dimming:    "
    "DC boost time:  "
    "Weather algo.:  "
    "NTP server.ip1: "
    "NTP server.ip2: "
    "NTP server.ip3: "
    "NTP server.ip4: "
    "Enable logging? "
    "Master 2 (Mas2):"
    "Mas2  on adjust:"
    "Mas2 off adjust:"
    "Firmware minor: "
    "Pulse rate:     "
    "----------------"
    "As remote ext.? "
    "Factory reset?  ";

/** Option maximum values (stored in progmem) */
const char op_max[] = {
  0,
  108,
  1,
  1,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  0,
  MAX_EXT_BOARDS,
  1,
  247,
  MAX_NUM_STATIONS,
  60,
  120,
  255,
  1,
  250,
  1,
  1,
  255,
  255,
  255,
  255,
  250,
  255,
  255,
  255,
  255,
  255,
  1,
  MAX_NUM_STATIONS,
  60,
  120,
  0,
  255,
  255,
  1,
  1
};

/** Option values (stored in RAM) */
byte OpenHome::options[] = {
  OS_FW_VERSION, // firmware version
  28, // default time zone: GMT-5
  1,  // 0: disable NTP sync, 1: enable NTP sync
  1,  // 0: use static ip, 1: use dhcp
  0,  // this and next 3 bytes define static ip
  0,
  0,
  0,
  0,  // this and next 3 bytes define static gateway ip
  0,
  0,
  0,
  144,// this and next byte define http port number
  31,
  OS_HW_VERSION,
  0,  // number of 8-station extension board. 0: no extension boards
  1,  // the option 'sequential' is now retired
  128,// station delay time (-59 minutes to 59 minutes).
  0,  // index of master station. 0: no master station
  0,  // master on time [0,60] seconds
  60, // master off time [-60,60] seconds
  0,  // sensor function (see SENSOR_TYPE macro defines)
  0,  // rain sensor type. 0: normally closed; 1: normally open.
  100,// water level (default 100%),
  1,  // device enable
  0,  // 1: ignore password; 0: use password
  0,  // device id
  150,// lcd contrast
  100,// lcd backlight
  50, // lcd dimming
  80, // boost time (only valid to DC and LATCH type)
  0,  // weather algorithm (0 means not using weather algorithm)
  50, // this and the next three bytes define the ntp server ip
  97,
  210,
  169,
  1,  // enable logging: 0: disable; 1: enable.
  0,  // index of master 2. 0: no master2 station
  0,
  60,
  OS_FW_MINOR, // firmware minor version
  100,// this and next byte define flow pulse rate (100x)
  0,
  0,  // set as remote extension
  0   // reset
};

/** Weekday strings (stored in progmem, for LCD display) */
static const char days_str[] PROGMEM =
  "Mon\0"
  "Tue\0"
  "Wed\0"
  "Thu\0"
  "Fri\0"
  "Sat\0"
  "Sun\0";

/** Calculate local time (UTC time plus time zone offset) */
time_t OpenHome::now_tz() {
  return now()+(int32_t)3600/4*(int32_t)(options[OPTION_TIMEZONE]-48);
}

#include "etherport.h"
#include <sys/reboot.h>
#include <stdlib.h>
#include "utils.h"
#include "server.h"

extern EthernetServer *m_server;
extern char ether_buffer[];

/** Initialize network with the given mac address and http port */
byte OpenHome::start_network() {
  unsigned int port = (unsigned int)(options[OPTION_HTTPPORT_1]<<8) + (unsigned int)options[OPTION_HTTPPORT_0];
#if defined(DEMO)
  port = 80;
#endif
  if(m_server)  {
    delete m_server;
    m_server = 0;
  }

  m_server = new EthernetServer(port);
  return m_server->begin();
}

/** Reboot controller */
void OpenHome::reboot_dev() {
#if defined(DEMO)
  // do nothing
#else
  sync(); // add sync to prevent file corruption
	reboot(RB_AUTOBOOT);
#endif
}

/** Launch update script */
void OpenHome::update_dev() {
  char cmd[1024];
  sprintf(cmd, "cd %s & ./updater.sh", get_runtime_path());
  system(cmd);
}

extern void flow_isr();
/** Initialize pins, controller variables, LCD */
void OpenHome::begin() {

  // shift register setup
  pinMode(PIN_SR_OE, OUTPUT);
  // pull shift register OE high to disable output
  digitalWrite(PIN_SR_OE, HIGH);
  pinMode(PIN_SR_LATCH, OUTPUT);
  digitalWrite(PIN_SR_LATCH, HIGH);

  pinMode(PIN_SR_CLOCK, OUTPUT);

#if defined(OSPI)
  pin_sr_data = PIN_SR_DATA;
  // detect RPi revision
  unsigned int rev = detect_rpi_rev();
  if (rev==0x0002 || rev==0x0003)
    pin_sr_data = PIN_SR_DATA_ALT;
  // if this is revision 1, use PIN_SR_DATA_ALT
  pinMode(pin_sr_data, OUTPUT);
#else
  pinMode(PIN_SR_DATA,  OUTPUT);
#endif

	// Reset all stations
  clear_all_station_bits();
  apply_all_station_bits();

  // pull shift register OE low to enable output
  digitalWrite(PIN_SR_OE, LOW);

  // Rain sensor port set up
  pinMode(PIN_RAINSENSOR, INPUT);

  // Set up sensors
  // OSPI and OSBO use external pullups
  attachInterrupt(PIN_FLOWSENSOR, "falling", flow_isr);

  // Default controller status variables
  // Static variables are assigned 0 by default
  // so only need to initialize non-zero ones
  status.enabled = 1;
  status.safe_reboot = 0;

  old_status = status;

  nvdata.sunrise_time = 360;  // 6:00am default sunrise
  nvdata.sunset_time = 1080;  // 6:00pm default sunset

  nboards = 1;
  nstations = 8;

  // set rf data pin
  pinMode(PIN_RF_DATA, OUTPUT);
  digitalWrite(PIN_RF_DATA, LOW);

  hw_type = HW_TYPE_AC;

  DEBUG_PRINTLN(get_runtime_path());
}

/** Apply all station bits
 * !!! This will activate/deactivate valves !!!
 */
void OpenHome::apply_all_station_bits() {
  digitalWrite(PIN_SR_LATCH, LOW);
  byte bid, s, sbits;

  // Shift out all station bit values
  // from the highest bit to the lowest
  for(bid=0;bid<=MAX_EXT_BOARDS;bid++) {
    if (status.enabled)
      sbits = station_bits[MAX_EXT_BOARDS-bid];
    else
      sbits = 0;

    for(s=0;s<8;s++) {
      digitalWrite(PIN_SR_CLOCK, LOW);
#if defined(OSPI) // if OSPI, use dynamically assigned pin_sr_data
      digitalWrite(pin_sr_data, (sbits & ((byte)1<<(7-s))) ? HIGH : LOW );
#else
      digitalWrite(PIN_SR_DATA, (sbits & ((byte)1<<(7-s))) ? HIGH : LOW );
#endif
      digitalWrite(PIN_SR_CLOCK, HIGH);
    }
  }

  digitalWrite(PIN_SR_LATCH, HIGH);

  // handle refresh of RF and remote stations
  // each time apply_all_station_bits is called
  // we refresh the station whose index is the current time modulo MAX_NUM_STATIONS
  static byte last_sid = 0;
  byte sid = now() % MAX_NUM_STATIONS;
  if (sid != last_sid) {  // avoid refreshing the same station twice in a roll
    last_sid = sid;
    bid=sid>>3;
    s=sid&0x07;
    switch_special_station(sid, (station_bits[bid]>>s)&0x01);
  }
}

/** Read rain sensor status */
void OpenHome::rainsensor_status() {
  // options[OPTION_RS_TYPE]: 0 if normally closed, 1 if normally open
  if(options[OPTION_SENSOR_TYPE]!=SENSOR_TYPE_RAIN) return;
  status.rain_sensed = (digitalRead(PIN_RAINSENSOR) == options[OPTION_RAINSENSOR_TYPE] ? 0 : 1);
}

/** Read the number of 8-station expansion boards */
// AVR has capability to detect number of expansion boards
int OpenHome::detect_exp() {
  return -1;
}

/** Convert hex code to ulong integer */
static ulong hex2ulong(byte *code, byte len) {
  char c;
  ulong v = 0;
  for(byte i=0;i<len;i++) {
    c = code[i];
    v <<= 4;
    if(c>='0' && c<='9') {
      v += (c-'0');
    } else if (c>='A' && c<='F') {
      v += 10 + (c-'A');
    } else if (c>='a' && c<='f') {
      v += 10 + (c-'a');
    } else {
      return 0;
    }
  }
  return v;
}

/** Parse RF code into on/off/timeing sections */
uint16_t OpenHome::parse_rfstation_code(RFStationData *data, ulong* on, ulong *off) {
  ulong v;
  v = hex2ulong(data->on, sizeof(data->on));
  if (!v) return 0;
  if (on) *on = v;
	v = hex2ulong(data->off, sizeof(data->off));
  if (!v) return 0;
  if (off) *off = v;
	v = hex2ulong(data->timing, sizeof(data->timing));
  if (!v) return 0;
  return v;
}

/** Get station name from NVM */
void OpenHome::get_station_name(byte sid, char tmp[]) {
  tmp[STATION_NAME_SIZE]=0;
  nvm_read_block(tmp, (void*)(ADDR_NVM_STN_NAMES+(int)sid*STATION_NAME_SIZE), STATION_NAME_SIZE);
}

/** Set station name to NVM */
void OpenHome::set_station_name(byte sid, char tmp[]) {
  tmp[STATION_NAME_SIZE]=0;
  nvm_write_block(tmp, (void*)(ADDR_NVM_STN_NAMES+(int)sid*STATION_NAME_SIZE), STATION_NAME_SIZE);
}

/** Save station attribute bits to NVM */
void OpenHome::station_attrib_bits_save(int addr, byte bits[]) {
  nvm_write_block(bits, (void*)addr, MAX_EXT_BOARDS+1);
}

/** Load all station attribute bits from NVM */
void OpenHome::station_attrib_bits_load(int addr, byte bits[]) {
  nvm_read_block(bits, (void*)addr, MAX_EXT_BOARDS+1);
}

/** Read one station attribute byte from NVM */
byte OpenHome::station_attrib_bits_read(int addr) {
  return nvm_read_byte((byte*)addr);
}

/** verify if a string matches password */
byte OpenHome::password_verify(char *pw) {
  byte *addr = (byte*)ADDR_NVM_PASSWORD;
  byte c1, c2;
  while(1) {
    if(addr == (byte*)ADDR_NVM_PASSWORD+MAX_USER_PASSWORD)
      c1 = 0;
    else
      c1 = nvm_read_byte(addr++);
    c2 = *pw++;
    if (c1==0 || c2==0)
      break;
    if (c1!=c2) {
      return 0;
    }
  }
  return (c1==c2) ? 1 : 0;
}

// ==================
// Schedule Functions
// ==================

/** Index of today's weekday (Monday is 0) */
byte OpenHome::weekday_today() {
  //return ((byte)weekday()+5)%7; // Time::weekday() assumes Sunday is 1
  return 0;
  // todo: is this function needed for RPI/BBB?
}

/** Switch special station */
void OpenHome::switch_special_station(byte sid, byte value) {
  // check station special bit
  if(station_attrib_bits_read(ADDR_NVM_STNSPE+(sid>>3))&(1<<(sid&0x07))) {
    // read station special data from sd card
    int stepsize=sizeof(StationSpecialData);
    read_from_file(stns_filename, tmp_buffer, stepsize, sid*stepsize);
    StationSpecialData *stn = (StationSpecialData *)tmp_buffer;
    // check station type
    if(stn->type==STN_TYPE_RF) {
      // transmit RF signal
      switch_rfstation((RFStationData *)stn->data, value);
    } else if(stn->type==STN_TYPE_REMOTE) {
      // request remote station
      switch_remotestation((RemoteStationData *)stn->data, value);
    } else if(stn->type==STN_TYPE_GPIO) {
      // set GPIO pin
      switch_gpiostation((GPIOStationData *)stn->data, value);
    } else if(stn->type==STN_TYPE_HTTP) {
      // send GET command
      switch_httpstation((HTTPStationData *)stn->data, value);
    }
  }
}

/** Set station bit
 * This function sets/resets the corresponding station bit variable
 * You have to call apply_all_station_bits next to apply the bits
 * (which results in physical actions of opening/closing valves).
 */
byte OpenHome::set_station_bit(byte sid, byte value) {
  byte *data = station_bits+(sid>>3);  // pointer to the station byte
  byte mask = (byte)1<<(sid&0x07); // mask
  if (value) {
    if((*data)&mask) return 0;  // if bit is already set, return no change
    else {
      (*data) = (*data) | mask;
      switch_special_station(sid, 1); // handle special stations
      return 1;
    }
  } else {
    if(!((*data)&mask)) return 0; // if bit is already reset, return no change
    else {
      (*data) = (*data) & (~mask);
      switch_special_station(sid, 0); // handle special stations
      return 255;
    }
  }
  return 0;
}

/** Clear all station bits */
void OpenHome::clear_all_station_bits() {
  byte sid;
  for(sid=0;sid<=MAX_NUM_STATIONS;sid++) {
    set_station_bit(sid, 0);
  }
}


int rf_gpio_fd = -1;

/** Transmit one RF signal bit */
void transmit_rfbit(ulong lenH, ulong lenL) {
  gpio_write(rf_gpio_fd, 1);
  delayMicrosecondsHard(lenH);
  gpio_write(rf_gpio_fd, 0);
  delayMicrosecondsHard(lenL);
}

/** Transmit RF signal */
void send_rfsignal(ulong code, ulong len) {
  ulong len3 = len * 3;
  ulong len31 = len * 31;
  for(byte n=0;n<15;n++) {
    int i=23;
    // send code
    while(i>=0) {
      if ((code>>i) & 1) {
        transmit_rfbit(len3, len);
      } else {
        transmit_rfbit(len, len3);
      }
      i--;
    };
    // send sync
    transmit_rfbit(len, len31);
  }
}

/** Switch RF station
 * This function takes a RF code,
 * parses it into signals and timing,
 * and sends it out through RF transmitter.
 */
void OpenHome::switch_rfstation(RFStationData *data, bool turnon) {
  ulong on, off;
  uint16_t length = parse_rfstation_code(data, &on, &off);
  // pre-open gpio file to minimize overhead
  rf_gpio_fd = gpio_fd_open(PIN_RF_DATA);
  send_rfsignal(turnon ? on : off, length);
  gpio_fd_close(rf_gpio_fd);
  rf_gpio_fd = -1;
}

/** Switch GPIO station
 * Special data for GPIO Station is three bytes of ascii decimal (not hex)
 * First two bytes are zero padded GPIO pin number.
 * Third byte is either 0 or 1 for active low (GND) or high (+5V) relays
 */
void OpenHome::switch_gpiostation(GPIOStationData *data, bool turnon) {
  byte gpio = (data->pin[0] - '0') * 10 + (data->pin[1] - '0');
  byte activeState = data->active - '0';

  pinMode(gpio, OUTPUT);
  if (turnon)
    digitalWrite(gpio, activeState);
  else
    digitalWrite(gpio, 1-activeState);
}

/** Callback function for remote station calls */
static void switchremote_callback(byte status, uint16_t off, uint16_t len) {
  /* do nothing */
}

/** Switch remote station
 * This function takes a remote station code,
 * parses it into remote IP, port, station index,
 * and makes a HTTP GET request.
 * The remote controller is assumed to have the same
 * password as the main controller
 */
void OpenHome::switch_remotestation(RemoteStationData *data, bool turnon) {
  EthernetClient client;

  uint8_t hisip[4];
  uint16_t hisport;
  ulong ip = hex2ulong(data->ip, sizeof(data->ip));
  hisip[0] = ip>>24;
  hisip[1] = (ip>>16)&0xff;
  hisip[2] = (ip>>8)&0xff;
  hisip[3] = ip&0xff;
  hisport = hex2ulong(data->port, sizeof(data->port));

  if (!client.connect(hisip, hisport)) {
    client.stop();
    return;
  }

  char *p = tmp_buffer + sizeof(RemoteStationData) + 1;
  BufferFiller bf = p;
  bf.emit_p(PSTR("GET /cm?pw=$E&sid=$D&en=$D&t=$D"),
            ADDR_NVM_PASSWORD,
            (int)hex2ulong(data->sid, sizeof(data->sid)),
            turnon, 2*MAX_NUM_STATIONS);  // MAX_NUM_STATIONS is the refresh cycle
  bf.emit_p(PSTR(" HTTP/1.0\r\nHOST: *\r\n\r\n"));

  client.write((uint8_t *)p, strlen(p));

  bzero(ether_buffer, ETHER_BUFFER_SIZE);

  time_t timeout = now() + 5; // 5 seconds timeout
  while(now() < timeout) {
    int len=client.read((uint8_t *)ether_buffer, ETHER_BUFFER_SIZE);
    if (len<=0) {
      if(!client.connected())
        break;
      else
        continue;
    }
    switchremote_callback(0, 0, ETHER_BUFFER_SIZE);
  }
  client.stop();
}

/** Callback function for http station calls */
static void switchhttp_callback(byte status, uint16_t off, uint16_t len) {
  /* do nothing */
}

/** Switch http station
 * This function takes an http station code,
 * parses it into a server name and two HTTP GET requests.
 */
void OpenHome::switch_httpstation(HTTPStationData *data, bool turnon) {

  char * server = strtok((char *)data->data, ",");
  char * port = strtok(NULL, ",");
  char * on_cmd = strtok(NULL, ",");
  char * off_cmd = strtok(NULL, ",");
  char * cmd = turnon ? on_cmd : off_cmd;

  EthernetClient client;
  struct hostent *host;

  host = gethostbyname(server);
  if (!host) {
    DEBUG_PRINT("can't resolve http station - ");
    DEBUG_PRINTLN(server);
    return;
  }

  if (!client.connect((uint8_t*)host->h_addr, atoi(port))) {
    client.stop();
    return;
  }

  char getBuffer[255];
  sprintf(getBuffer, "GET /%s HTTP/1.0\r\nHOST: %s\r\n\r\n", cmd, host->h_name);
  client.write((uint8_t *)getBuffer, strlen(getBuffer));

  bzero(ether_buffer, ETHER_BUFFER_SIZE);

  time_t timeout = now() + 5; // 5 seconds timeout
  while(now() < timeout) {
    int len=client.read((uint8_t *)ether_buffer, ETHER_BUFFER_SIZE);
    if (len<=0) {
      if(!client.connected())
        break;
      else
        continue;
    }
    switchhttp_callback(0, 0, ETHER_BUFFER_SIZE);
  }

  client.stop();
}

/** Setup function for options */
void OpenHome::options_setup() {

  // add 0.25 second delay to allow nvm to stablize
  delay(250);

  byte curr_ver = nvm_read_byte((byte*)(ADDR_NVM_OPTIONS+OPTION_FW_VERSION));

  // check reset condition: either firmware version has changed, or reset flag is up
  // if so, trigger a factory reset
  if (curr_ver != OS_FW_VERSION || nvm_read_byte((byte*)(ADDR_NVM_OPTIONS+OPTION_RESET))==0xAA)  {
    DEBUG_PRINT("Resetting Options...");

    // ======== Reset NVM data ========
    int i, sn;

    // 0. wipe out nvm
    for(i=0;i<TMP_BUFFER_SIZE;i++) tmp_buffer[i]=0;
    for(i=0;i<NVM_SIZE;i+=TMP_BUFFER_SIZE) {
      int nbytes = ((NVM_SIZE-i)>TMP_BUFFER_SIZE)?TMP_BUFFER_SIZE:(NVM_SIZE-i);
      nvm_write_block(tmp_buffer, (void*)i, nbytes);
    }

    // 1. write non-volatile controller status
    nvdata_save();

    // 2. write string parameters
    nvm_write_block(DEFAULT_PASSWORD, (void*)ADDR_NVM_PASSWORD, strlen(DEFAULT_PASSWORD)+1);
    nvm_write_block(DEFAULT_LOCATION, (void*)ADDR_NVM_LOCATION, strlen(DEFAULT_LOCATION)+1);
    nvm_write_block(DEFAULT_JAVASCRIPT_URL, (void*)ADDR_NVM_JAVASCRIPTURL, strlen(DEFAULT_JAVASCRIPT_URL)+1);
    nvm_write_block(DEFAULT_WEATHER_URL, (void*)ADDR_NVM_WEATHERURL, strlen(DEFAULT_WEATHER_URL)+1);
    nvm_write_block(DEFAULT_WEATHER_KEY, (void*)ADDR_NVM_WEATHER_KEY, strlen(DEFAULT_WEATHER_KEY)+1);

    // 3. reset station names and special attributes, default Sxx
    tmp_buffer[0]='S';
    tmp_buffer[3]=0;
    for(i=ADDR_NVM_STN_NAMES, sn=1; i<ADDR_NVM_MAS_OP; i+=STATION_NAME_SIZE, sn++) {
      tmp_buffer[1]='0'+(sn/10);
      tmp_buffer[2]='0'+(sn%10);
      nvm_write_block(tmp_buffer, (void*)i, strlen(tmp_buffer)+1);
    }

    tmp_buffer[0]=STN_TYPE_STANDARD;
    tmp_buffer[1]='0';
    tmp_buffer[2]=0;
    int stepsize=sizeof(StationSpecialData);
    for(i=0;i<MAX_NUM_STATIONS;i++) {
        write_to_file(stns_filename, tmp_buffer, stepsize, i*stepsize, false);
    }
    // 4. reset station attribute bits
    // since we wiped out nvm, only non-zero attributes need to be initialized
    for(i=0;i<MAX_EXT_BOARDS+1;i++) {
      tmp_buffer[i]=0xff;
    }
    nvm_write_block(tmp_buffer, (void*)ADDR_NVM_MAS_OP, MAX_EXT_BOARDS+1);
    nvm_write_block(tmp_buffer, (void*)ADDR_NVM_STNSEQ, MAX_EXT_BOARDS+1);

    // 5. delete sd file
    remove_file(wtopts_filename);

    // 6. write options
    options_save(); // write default option values

    //======== END OF NVM RESET CODE ========

    // restart after resetting NVM.
    delay(500);
  }

  {
    // load ram parameters from nvm
    // load options
    options_load();

    // load non-volatile controller data
    nvdata_load();
  }
}

/** Load non-volatile controller status data from internal NVM */
void OpenHome::nvdata_load() {
  nvm_read_block(&nvdata, (void*)ADDR_NVM_NVCONDATA, sizeof(NVConData));
  old_status = status;
}

/** Save non-volatile controller status data to internal NVM */
void OpenHome::nvdata_save() {
  nvm_write_block(&nvdata, (void*)ADDR_NVM_NVCONDATA, sizeof(NVConData));
}

/** Load options from internal NVM */
void OpenHome::options_load() {
  nvm_read_block(tmp_buffer, (void*)ADDR_NVM_OPTIONS, NUM_OPTIONS);
  for (byte i=0; i<NUM_OPTIONS; i++) {
    options[i] = tmp_buffer[i];
  }
  nboards = options[OPTION_EXT_BOARDS]+1;
  nstations = nboards * 8;
  status.enabled = options[OPTION_DEVICE_ENABLE];
  options[OPTION_FW_MINOR] = OS_FW_MINOR;
}

/** Save options to internal NVM */
void OpenHome::options_save() {
  // save options in reverse order so version number is written the last
  for (int i=NUM_OPTIONS-1; i>=0; i--) {
    tmp_buffer[i] = options[i];
  }
  nvm_write_block(tmp_buffer, (void*)ADDR_NVM_OPTIONS, NUM_OPTIONS);
  nboards = options[OPTION_EXT_BOARDS]+1;
  nstations = nboards * 8;
  status.enabled = options[OPTION_DEVICE_ENABLE];
}

// ==============================
// Controller Operation Functions
// ==============================

/** Enable controller operation */
void OpenHome::enable() {
  status.enabled = 1;
  options[OPTION_DEVICE_ENABLE] = 1;
  options_save();
}

/** Disable controller operation */
void OpenHome::disable() {
  status.enabled = 0;
  options[OPTION_DEVICE_ENABLE] = 0;
  options_save();
}

/** Start rain delay */
void OpenHome::raindelay_start() {
  status.rain_delayed = 1;
  nvdata_save();
}

/** Stop rain delay */
void OpenHome::raindelay_stop() {
  status.rain_delayed = 0;
  nvdata.rd_stop_time = 0;
  nvdata_save();
}