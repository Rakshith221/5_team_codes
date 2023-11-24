#include "contiki.h"
#include "dev/leds.h"
#include "dev/sht11/sht11-sensor.h"
#include "jsontree.h"
#include "json-ws.h"
#include <stdio.h>
#include <stdlib.h>

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

PROCESS(websense_process, "Websense (sky)");
AUTOSTART_PROCESSES(&websense_process);

/*---------------------------------------------------------------------------*/
static CC_INLINE int
get_lat(void)
{
  return (rand() % 180 + 90);  
}



static CC_INLINE int
get_long(void)
{
  return (rand() % 360 + 180);  
}

static CC_INLINE int
get_power(void)
{
  return rand() % 2;
  
}

static CC_INLINE int
get_equipment_id(void)
{
    return rand() % 1000;
  
}


static CC_INLINE int
get_rfid(void)
{
  return rand() % 3;
  
}



/*---------------------------------------------------------------------------*/
static int
output_lat(struct jsontree_context *path)
{
  char buf[5];
  snprintf(buf, sizeof(buf), "%3d", get_lat());
  jsontree_write_atom(path, buf);
  return 0;
}

static int
output_long(struct jsontree_context *path)
{
  char buf[5];
  snprintf(buf, sizeof(buf), "%3d", get_long());
  jsontree_write_atom(path, buf);
  return 0;
}

static int
output_power(struct jsontree_context *path)
{
  char buf[5];
  snprintf(buf, sizeof(buf), "%3d", get_power());
  jsontree_write_atom(path, buf);
  return 0;
}

static int
output_equipment_id(struct jsontree_context *path)
{
  char buf[5];
  snprintf(buf, sizeof(buf), "%3d", get_equipment_id());
  jsontree_write_atom(path, buf);
  return 0;
}

static int
output_rfid(struct jsontree_context *path)
{
  char buf[5];
  snprintf(buf, sizeof(buf), "%3d", get_rfid());
  jsontree_write_atom(path, buf);
  return 0;
}

static struct jsontree_callback get_lat_callback =
  JSONTREE_CALLBACK(output_lat, NULL);
  
static struct jsontree_callback get_long_callback =
  JSONTREE_CALLBACK(output_long, NULL);
 
static struct jsontree_callback get_power_callback =
  JSONTREE_CALLBACK(output_power, NULL);

static struct jsontree_callback equipment_id_callback =
  JSONTREE_CALLBACK(output_equipment_id, NULL);

static struct jsontree_callback rfid_callback =
  JSONTREE_CALLBACK(output_rfid, NULL);
 
/*---------------------------------------------------------------------------*/

static struct jsontree_string desc = JSONTREE_STRING("Tmote Sky");
static struct jsontree_string temp_unit = JSONTREE_STRING("");
static struct jsontree_string light_unit = JSONTREE_STRING("");
static struct jsontree_string air_unit = JSONTREE_STRING("");
static struct jsontree_string humid_unit = JSONTREE_STRING("");
static struct jsontree_string rfid_unit = JSONTREE_STRING("");

JSONTREE_OBJECT(node_tree,
                JSONTREE_PAIR("node-type", &desc),
                JSONTREE_PAIR("time", &json_time_callback));

JSONTREE_OBJECT(lat_sensor_tree,
                JSONTREE_PAIR("unit", &temp_unit),
                JSONTREE_PAIR("value", &get_lat_callback));
                
JSONTREE_OBJECT(long_sensor_tree,
                JSONTREE_PAIR("unit", &light_unit),
                JSONTREE_PAIR("value", &get_long_callback)); 

JSONTREE_OBJECT(power_sensor_tree,
                JSONTREE_PAIR("unit", &air_unit),
                JSONTREE_PAIR("value", &get_power_callback)); 

JSONTREE_OBJECT(equipment_id_sensor_tree,
                JSONTREE_PAIR("unit", &humid_unit),
                JSONTREE_PAIR("value", &equipment_id_callback));   

JSONTREE_OBJECT(rfid_sensor_tree,
                JSONTREE_PAIR("unit", &rfid_unit),
                JSONTREE_PAIR("value", &rfid_callback));   

JSONTREE_OBJECT(rsc_tree,
                JSONTREE_PAIR("latitude", &lat_sensor_tree),
                JSONTREE_PAIR("longitude", &long_sensor_tree),
                JSONTREE_PAIR("power", &power_sensor_tree),
                JSONTREE_PAIR("equipment_id", &equipment_id_sensor_tree),
                JSONTREE_PAIR("rfid", &rfid_sensor_tree),
                JSONTREE_PAIR("leds", &json_leds_callback));

/* complete node tree */
JSONTREE_OBJECT(tree,
                JSONTREE_PAIR("node", &node_tree),
                JSONTREE_PAIR("rsc", &rsc_tree),
                JSONTREE_PAIR("cfg", &json_subscribe_callback));

/*---------------------------------------------------------------------------*/
/* for cosm plugin */
#if WITH_COSM
/* set COSM value callback to be the temp sensor */
struct jsontree_callback cosm_value_callback =
  JSONTREE_CALLBACK(output_lat, output_long,output_power, output_equipment_id, output_rfid, NULL);
#endif

PROCESS_THREAD(websense_process, ev, data)
{
  static struct etimer timer;

  PROCESS_BEGIN();

  json_ws_init(&tree);

  SENSORS_ACTIVATE(sht11_sensor);

  json_ws_set_callback("rsc");

  while(1) {
    /* Alive indication with the LED */
    etimer_set(&timer, CLOCK_SECOND * 2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    leds_on(LEDS_RED);
    /*etimer_set(&timer, CLOCK_SECOND / 8);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    leds_off(LEDS_RED);
    */
  }

  PROCESS_END();
}
