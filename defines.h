/* OpenHome Firmware
 * Copyright (C) 2015 by Charles Remeikas
 *
 * OpenHome macro defines and hardware pin assignments
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

#ifndef _DEFINES_H
#define _DEFINES_H

#define TMP_BUFFER_SIZE      128    // scratch buffer size

/** Firmware version, hardware version, and maximal values */
#define OS_FW_VERSION  216  // Firmware version: 216 means 2.1.6
                            // if this number is different from the one stored in non-volatile memory
                            // a device reset will be automatically triggered

#define OS_FW_MINOR      3  // Firmware minor version

/** Hardware version base numbers */
#define OS_HW_VERSION_BASE   0x00
#define OSPI_HW_VERSION_BASE 0x40
#define OSBO_HW_VERSION_BASE 0x80
#define SIM_HW_VERSION_BASE  0xC0

/** Hardware type macro defines */
#define HW_TYPE_AC           0xAC   // standard 24VAC for 24VAC solenoids only, with triacs
#define HW_TYPE_DC           0xDC   // DC powered, for both DC and 24VAC solenoids, with boost converter and MOSFETs
#define HW_TYPE_LATCH        0x1A   // DC powered, for DC latching solenoids only, with boost converter and H-bridges

/** File names */
#define WEATHER_OPTS_FILENAME "wtopts.txt"    // weather options file
#define STATION_ATTR_FILENAME "stns.dat"      // station attributes data file
#define STATION_SPECIAL_DATA_SIZE  (TMP_BUFFER_SIZE - 8)

#define FLOWCOUNT_RT_WINDOW   30    // flow count window (for computing real-time flow rate), 30 seconds

/** Station type macro defines */
#define STN_TYPE_STANDARD    0x00
#define STN_TYPE_RF          0x01
#define STN_TYPE_REMOTE      0x02
#define STN_TYPE_GPIO        0x03	// Support for raw connection of station to GPIO pin
#define STN_TYPE_HTTP        0x04	// Support for HTTP Get connection
#define STN_TYPE_OTHER       0xFF

/** Sensor type macro defines */
#define SENSOR_TYPE_NONE    0x00
#define SENSOR_TYPE_RAIN    0x01
#define SENSOR_TYPE_FLOW    0x02
#define SENSOR_TYPE_OTHER   0xFF

// These are kept the same as AVR for compatibility reasons
// But they can be increased if needed
#define NVM_FILENAME        "nvm.dat" // for RPI/BBB, nvm data is stored in a file

#define MAX_EXT_BOARDS    6  // maximum number of exp. boards (each expands 8 stations)
#define MAX_NUM_STATIONS  ((1+MAX_EXT_BOARDS)*8)  // maximum number of stations

#define NVM_SIZE            4096
#define STATION_NAME_SIZE   24    // maximum number of characters in each station name

#define MAX_PROGRAMDATA     2438  // program data
#define MAX_NVCONDATA       12     // non-volatile controller data
#define MAX_USER_PASSWORD   36    // user password
#define MAX_LOCATION        48    // location string
#define MAX_JAVASCRIPTURL   48    // javascript url
#define MAX_WEATHERURL      48    // weather script url
#define MAX_WEATHER_KEY     24    // weather api key

/** NVM data addresses */
#define ADDR_NVM_PROGRAMS      (0)   // program starting address
#define ADDR_NVM_NVCONDATA     (ADDR_NVM_PROGRAMS+MAX_PROGRAMDATA)
#define ADDR_NVM_PASSWORD      (ADDR_NVM_NVCONDATA+MAX_NVCONDATA)
#define ADDR_NVM_LOCATION      (ADDR_NVM_PASSWORD+MAX_USER_PASSWORD)
#define ADDR_NVM_JAVASCRIPTURL (ADDR_NVM_LOCATION+MAX_LOCATION)
#define ADDR_NVM_WEATHERURL    (ADDR_NVM_JAVASCRIPTURL+MAX_JAVASCRIPTURL)
#define ADDR_NVM_WEATHER_KEY   (ADDR_NVM_WEATHERURL+MAX_WEATHERURL)
#define ADDR_NVM_STN_NAMES     (ADDR_NVM_WEATHER_KEY+MAX_WEATHER_KEY)
#define ADDR_NVM_MAS_OP        (ADDR_NVM_STN_NAMES+MAX_NUM_STATIONS*STATION_NAME_SIZE) // master op bits
#define ADDR_NVM_IGNRAIN       (ADDR_NVM_MAS_OP+(MAX_EXT_BOARDS+1))  // ignore rain bits
#define ADDR_NVM_MAS_OP_2      (ADDR_NVM_IGNRAIN+(MAX_EXT_BOARDS+1)) // master2 op bits
#define ADDR_NVM_STNDISABLE    (ADDR_NVM_MAS_OP_2+(MAX_EXT_BOARDS+1))// station disable bits
#define ADDR_NVM_STNSEQ        (ADDR_NVM_STNDISABLE+(MAX_EXT_BOARDS+1))// station sequential bits
#define ADDR_NVM_STNSPE        (ADDR_NVM_STNSEQ+(MAX_EXT_BOARDS+1)) // station special bits (i.e. non-standard stations)
#define ADDR_NVM_OPTIONS       (ADDR_NVM_STNSPE+(MAX_EXT_BOARDS+1))  // options

/** Default password, location string, weather key, script urls */
#define DEFAULT_PASSWORD          "Undine12"
#define DEFAULT_LOCATION          "Clearwater,FL"
#define DEFAULT_WEATHER_KEY       ""
#define DEFAULT_JAVASCRIPT_URL    "https://ui.opensprinkler.com/js"
#define DEFAULT_WEATHER_URL       "weather.opensprinkler.com"

/** Macro define of each option
  * Refer to OpenHome.cpp for details on each option
  */
typedef enum {
  OPTION_FW_VERSION = 0,
  OPTION_TIMEZONE,
  OPTION_USE_NTP,
  OPTION_USE_DHCP,
  OPTION_STATIC_IP1,
  OPTION_STATIC_IP2,
  OPTION_STATIC_IP3,
  OPTION_STATIC_IP4,
  OPTION_GATEWAY_IP1,
  OPTION_GATEWAY_IP2,
  OPTION_GATEWAY_IP3,
  OPTION_GATEWAY_IP4,
  OPTION_HTTPPORT_0,
  OPTION_HTTPPORT_1,
  OPTION_HW_VERSION,
  OPTION_EXT_BOARDS,
  OPTION_SEQUENTIAL_RETIRED,
  OPTION_STATION_DELAY_TIME,
  OPTION_MASTER_STATION,
  OPTION_MASTER_ON_ADJ,
  OPTION_MASTER_OFF_ADJ,
  OPTION_SENSOR_TYPE,
  OPTION_RAINSENSOR_TYPE,
  OPTION_WATER_PERCENTAGE,
  OPTION_DEVICE_ENABLE,
  OPTION_IGNORE_PASSWORD,
  OPTION_DEVICE_ID,
  OPTION_LCD_CONTRAST,
  OPTION_LCD_BACKLIGHT,
  OPTION_LCD_DIMMING,
  OPTION_BOOST_TIME,
  OPTION_USE_WEATHER,
  OPTION_NTP_IP1,
  OPTION_NTP_IP2,
  OPTION_NTP_IP3,
  OPTION_NTP_IP4,
  OPTION_ENABLE_LOGGING,
  OPTION_MASTER_STATION_2,
  OPTION_MASTER_ON_ADJ_2,
  OPTION_MASTER_OFF_ADJ_2,
  OPTION_FW_MINOR,
  OPTION_PULSE_RATE_0,
  OPTION_PULSE_RATE_1,
  OPTION_REMOTE_EXT_MODE,
  OPTION_RESET,
  NUM_OPTIONS	// total number of options
} OS_OPTION_t;

/** Log Data Type */
#define LOGDATA_STATION    0x00
#define LOGDATA_RAINSENSE  0x01
#define LOGDATA_RAINDELAY  0x02
#define LOGDATA_WATERLEVEL 0x03
#define LOGDATA_FLOWSENSE  0x04

#undef OS_HW_VERSION

#define PIN_FREE_LIST		{5,6,7,8,9,10,11,12,13,16,18,19,20,21,23,24,25,26}



#if defined(PINE)
  #define GPIO_ZONE_1 230
  #define GPIO_ZONE_2 69
  #define GPIO_ZONE_3 73
  #define GPIO_ZONE_4 80
#else
  #define GPIO_ZONE_1 0
  #define GPIO_ZONE_2 0
  #define GPIO_ZONE_3 0
  #define GPIO_ZONE_4 0
#endif


#define ETHER_BUFFER_SIZE   16384

#define DEBUG_BEGIN(x)          {}  /** Serial debug functions */
#define ENABLE_DEBUG
#if defined(ENABLE_DEBUG)
  inline  void DEBUG_PRINT(int x) {printf("%d", x);}
  inline  void DEBUG_PRINT(const char*s) {printf("%s", s);}
  #define DEBUG_PRINTLN(x)        {DEBUG_PRINT(x);printf("\n");}
#else
  #define DEBUG_PRINT(x) {}
  #define DEBUG_PRINTLN(x) {}
#endif

inline void itoa(int v,char *s,int b)   {sprintf(s,"%d",v);}
inline void ultoa(unsigned long v,char *s,int b) {sprintf(s,"%lu",v);}
#define now()       time(0)

/** Re-define avr-specific (e.g. PGM) types to use standard types */
#define pgm_read_byte(x) *(x)
#define PSTR(x)      x
#define strcat_P     strcat
#define strcpy_P     strcpy
#define PROGMEM
typedef const char* PGM_P;
typedef unsigned char   uint8_t;
typedef short           int16_t;
typedef unsigned short  uint16_t;
typedef bool boolean;

#define DISPLAY_MSG_MS      2000  // message display time (milliseconds)

typedef unsigned char byte;
typedef unsigned long ulong;

#endif  // _DEFINES_H


