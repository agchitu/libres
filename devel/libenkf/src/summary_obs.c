/**
   See the overview documentation of the observation system in enkf_obs.c
*/
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <util.h>
#include <stdio.h>
#include <summary_obs.h>
#include <obs_data.h>
#include <meas_matrix.h>
#include <summary.h>
#include <active_list.h>


#define SUMMARY_OBS_TYPE_ID 66103


struct summary_obs_struct {
  UTIL_TYPE_ID_DECLARATION;
  char    * summary_key;    /** The observation, in summary.x syntax, e.g. GOPR:FIELD.    */

  double    value;          /** Observation value. */
  double    std;            /** Standard deviation of observation. */
};





/**
  This function allocates a summary_obs instance. The summary_key
  string should be of the format used by the summary.x program.
  E.g., WOPR:P4 would condition on WOPR in well P4.

  Observe that this format is currently *not* checked before the actual
  observation time.

  TODO
  Should check summary_key on alloc.
*/
summary_obs_type * summary_obs_alloc(  
  const char   * summary_key,
  double value ,
  double std)
{
  summary_obs_type * obs = util_malloc(sizeof * obs , __func__);
  UTIL_TYPE_ID_INIT( obs , SUMMARY_OBS_TYPE_ID )

  obs->summary_key   = util_alloc_string_copy(summary_key);
  obs->value         = value;
  obs->std           = std;
  
  return obs;
}


static UTIL_SAFE_CAST_FUNCTION_CONST(summary_obs   , SUMMARY_OBS_TYPE_ID);
static UTIL_SAFE_CAST_FUNCTION(summary_obs   , SUMMARY_OBS_TYPE_ID);
UTIL_IS_INSTANCE_FUNCTION(summary_obs , SUMMARY_OBS_TYPE_ID);


void summary_obs_free(summary_obs_type * summary_obs) {
  free(summary_obs->summary_key);
  free(summary_obs);
}







const char * summary_obs_get_summary_key(const summary_obs_type * summary_obs)
{
  return summary_obs->summary_key;
}


/**
   Hardcodes an assumption that the size of summary data|observations
   is always one; i.e. PARTLY_ACTIVE and ALL_ACTIVE are treated in the
   same manner.
*/
void summary_obs_get_observations(const summary_obs_type * summary_obs,
				  int                      restart_nr,
				  obs_data_type          * obs_data,
				  const active_list_type * __active_list) {

  active_mode_type active_mode = active_list_get_mode( __active_list );
  if ((active_mode == ALL_ACTIVE) || (active_mode == PARTLY_ACTIVE)) {
    char * key = util_alloc_sprintf( "%s (%d)" , summary_obs->summary_key, restart_nr);
    obs_data_add(obs_data , summary_obs->value , summary_obs->std , key);
    free( key );
  }
}



void summary_obs_measure(const summary_obs_type * obs, const summary_type * summary, int report_step , meas_vector_type * meas_vector , const active_list_type * __active_list) {

  active_mode_type active_mode = active_list_get_mode( __active_list );
  if (active_mode == ALL_ACTIVE) 
    meas_vector_add(meas_vector , summary_get(summary));
  else if (active_mode == PARTLY_ACTIVE) {
    int active_size = active_list_get_active_size( __active_list );
    if (active_size == 1)
      meas_vector_add(meas_vector , summary_get(summary));
  }
}


 

double summary_obs_chi2(const summary_obs_type * obs,
			const summary_type     * summary) {
  double x = (summary_get(summary) - obs->value) / obs->std;
  return x*x;
}






void summary_obs_user_get(const summary_obs_type * summary_obs , const char * index_key , double * value , double * std, bool * valid) {
  *valid = true;
  *value = summary_obs->value;
  *std   = summary_obs->std;
}



/*****************************************************************/

VOID_FREE(summary_obs)
VOID_GET_OBS(summary_obs)
VOID_USER_GET_OBS(summary_obs)
VOID_MEASURE(summary_obs , summary)
VOID_CHI2(summary_obs , summary)
