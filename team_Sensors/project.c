#include "contiki.h"
#include "dev/leds.h"
#include "dev/sht11/sht11-sensor.h"
#include "jsontree.h"
#include "json-ws.h"
#include <stdio.h>

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
get_temp(void)
{
  return ((sht11_sensor.value(SHT11_SENSOR_TEMP) / 10) - 396) / 10;
}



static CC_INLINE int
get_light(void)
{
  static int c = 0;  
  int max_cycle = 10;
  c = (c + 5) % max_cycle;
  return c;
  
}

static CC_INLINE int
get_air(void)
{
  static int c = 0;  
  int max_cycle = 100;
  c = (c + 50) % max_cycle;
  return c;
  
}

static CC_INLINE int
get_humid(void)
{
  static int c = 0;  
  int max_cycle = 100;
  c = (c + 20) % max_cycle;
  return c;
  
}


/*---------------------------------------------------------------------------*/
static int
output_temp(struct jsontree_context *path)
{
  char buf[5];
  snprintf(buf, sizeof(buf), "%3d", get_temp());
  jsontree_write_atom(path, buf);
  return 0;
}

static int
output_light(struct jsontree_context *path)
{
  char buf[5];
  snprintf(buf, sizeof(buf), "%3d", get_light());
  jsontree_write_atom(path, buf);
  return 0;
}

static int
output_air(struct jsontree_context *path)
{
  char buf[5];
  snprintf(buf, sizeof(buf), "%3d", get_air());
  jsontree_write_atom(path, buf);
  return 0;
}

static int
output_humid(struct jsontree_context *path)
{
  char buf[5];
  snprintf(buf, sizeof(buf), "%3d", get_humid());
  jsontree_write_atom(path, buf);
  return 0;
}

static struct jsontree_callback temp_sensor_callback =
  JSONTREE_CALLBACK(output_temp, NULL);
  
static struct jsontree_callback light_sensor_callback =
  JSONTREE_CALLBACK(output_light, NULL);
 
static struct jsontree_callback air_sensor_callback =
  JSONTREE_CALLBACK(output_air, NULL);

static struct jsontree_callback humid_sensor_callback =
  JSONTREE_CALLBACK(output_humid, NULL);
 
/*---------------------------------------------------------------------------*/

static struct jsontree_string desc = JSONTREE_STRING("Tmote Sky");
static struct jsontree_string temp_unit = JSONTREE_STRING("Celcius");
static struct jsontree_string light_unit = JSONTREE_STRING("Lux");
static struct jsontree_string air_unit = JSONTREE_STRING("PPM");
static struct jsontree_string humid_unit = JSONTREE_STRING("%");

JSONTREE_OBJECT(node_tree,
                JSONTREE_PAIR("node-type", &desc),
                JSONTREE_PAIR("time", &json_time_callback));

JSONTREE_OBJECT(temp_sensor_tree,
                JSONTREE_PAIR("unit", &temp_unit),
                JSONTREE_PAIR("value", &temp_sensor_callback));
                
JSONTREE_OBJECT(light_sensor_tree,
                JSONTREE_PAIR("unit", &light_unit),
                JSONTREE_PAIR("value", &light_sensor_callback)); 

JSONTREE_OBJECT(air_sensor_tree,
                JSONTREE_PAIR("unit", &air_unit),
                JSONTREE_PAIR("value", &air_sensor_callback)); 

JSONTREE_OBJECT(humid_sensor_tree,
                JSONTREE_PAIR("unit", &humid_unit),
                JSONTREE_PAIR("value", &humid_sensor_callback));     

JSONTREE_OBJECT(rsc_tree,
                JSONTREE_PAIR("temperature", &temp_sensor_tree),
                JSONTREE_PAIR("light", &light_sensor_tree),
                JSONTREE_PAIR("air-quality", &air_sensor_tree),
                JSONTREE_PAIR("humidity", &humid_sensor_tree),
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
  JSONTREE_CALLBACK(output_temp, output_light,output_air, output_humid, NULL);
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
