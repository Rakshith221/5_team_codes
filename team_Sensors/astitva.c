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
get_bpm(void)
{
  return (rand() % 200 + 20);
}



static CC_INLINE int
get_hrv(void)
{
  return (rand() % 50 - 50);
  
}


/*---------------------------------------------------------------------------*/
static int
output_bpm(struct jsontree_context *path)
{
  char buf[5];
  snprintf(buf, sizeof(buf), "%3d", get_bpm());
  jsontree_write_atom(path, buf);
  return 0;
}

static int
output_hrv(struct jsontree_context *path)
{
  char buf[5];
  snprintf(buf, sizeof(buf), "%3d", get_hrv());
  jsontree_write_atom(path, buf);
  return 0;
}

static struct jsontree_callback bpm_sensor_callback =
  JSONTREE_CALLBACK(output_bpm, NULL);
  
static struct jsontree_callback hrv_sensor_callback =
  JSONTREE_CALLBACK(output_hrv, NULL);

/*---------------------------------------------------------------------------*/

static struct jsontree_string desc = JSONTREE_STRING("Tmote Sky");
static struct jsontree_string temp_unit = JSONTREE_STRING("Celcius");
static struct jsontree_string light_unit = JSONTREE_STRING("Lux");

JSONTREE_OBJECT(node_tree,
                JSONTREE_PAIR("node-type", &desc),
                JSONTREE_PAIR("time", &json_time_callback));

JSONTREE_OBJECT(bpm_sensor_tree,
                JSONTREE_PAIR("unit", &temp_unit),
                JSONTREE_PAIR("value", &bpm_sensor_callback));
                
JSONTREE_OBJECT(hrv_sensor_tree,
                JSONTREE_PAIR("unit", &light_unit),
                JSONTREE_PAIR("value", &hrv_sensor_callback)); 
   

JSONTREE_OBJECT(rsc_tree,
                JSONTREE_PAIR("bpm", &bpm_sensor_tree),
                JSONTREE_PAIR("hrv", &hrv_sensor_tree),
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
  JSONTREE_CALLBACK(output_bpm, output_hrv, NULL);
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
