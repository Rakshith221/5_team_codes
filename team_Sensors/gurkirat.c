#include "contiki.h"
#include "dev/leds.h"
#include "dev/sht11/sht11-sensor.h"
#include "jsontree.h"
#include "json-ws.h"
#include <stdlib.h>
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
get_quality(void)
{
  return rand() % 100;
}



static CC_INLINE int
get_snoring(void)
{
  static int c = 0;  
  int max_cycle = 10;
  c = (c + 5) % max_cycle;
  return c;
  
}

static CC_INLINE int
get_apnea(void)
{
  static int c = 0;  
  int max_cycle = 100;
  c = (c + 50) % max_cycle;
  return c;
  
}


/*---------------------------------------------------------------------------*/
static int
output_quality(struct jsontree_context *path)
{
  char buf[5];
  snprintf(buf, sizeof(buf), "%3d", get_quality());
  jsontree_write_atom(path, buf);
  return 0;
}

static int
output_snoring(struct jsontree_context *path)
{
  char buf[5];
  snprintf(buf, sizeof(buf), "%3d", get_snoring());
  jsontree_write_atom(path, buf);
  return 0;
}

static int
output_apnea(struct jsontree_context *path)
{
  char buf[5];
  snprintf(buf, sizeof(buf), "%3d", get_apnea());
  jsontree_write_atom(path, buf);
  return 0;
}

static struct jsontree_callback quality_sensor_callback =
  JSONTREE_CALLBACK(output_quality, NULL);
  
static struct jsontree_callback snoring_sensor_callback =
  JSONTREE_CALLBACK(output_snoring, NULL);
 
static struct jsontree_callback apnea_sensor_callback =
  JSONTREE_CALLBACK(output_apnea, NULL);
 
/*---------------------------------------------------------------------------*/

static struct jsontree_string desc = JSONTREE_STRING("Tmote Sky");
static struct jsontree_string temp_unit = JSONTREE_STRING("");
static struct jsontree_string light_unit = JSONTREE_STRING("");
static struct jsontree_string air_unit = JSONTREE_STRING("");

JSONTREE_OBJECT(node_tree,
                JSONTREE_PAIR("node-type", &desc),
                JSONTREE_PAIR("time", &json_time_callback));

JSONTREE_OBJECT(quality_sensor_tree,
                JSONTREE_PAIR("unit", &temp_unit),
                JSONTREE_PAIR("value", &quality_sensor_callback));
                
JSONTREE_OBJECT(snoring_sensor_tree,
                JSONTREE_PAIR("unit", &light_unit),
                JSONTREE_PAIR("value", &snoring_sensor_callback)); 

JSONTREE_OBJECT(apnea_sensor_tree,
                JSONTREE_PAIR("unit", &air_unit),
                JSONTREE_PAIR("value", &apnea_sensor_callback)); 

    

JSONTREE_OBJECT(rsc_tree,
                JSONTREE_PAIR("quality", &quality_sensor_tree),
                JSONTREE_PAIR("snoring", &snoring_sensor_tree),
                JSONTREE_PAIR("apnea", &apnea_sensor_tree),
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
  JSONTREE_CALLBACK(output_quality, output_snoring,output_apnea, NULL);
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
