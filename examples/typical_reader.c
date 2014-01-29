#include <cxi.h>
#include <stdlib.h>

/* We'll skimp on the error checking for the sake of clarity */
int main(void){
  char * filename = "../data/typical_raw.cxi";
  /* Open the CXI file for reading */
  CXI_File * file = cxi_open_file(filename,"r");
  if(!file) return -1;
  
  /* Check that the CXI version is at least 1.10 */
  if(file->cxi_version < 110){
    printf("Warning: CXI version is too low %d\n",file->cxi_version);
  }
  /* /entry_1 */
  CXI_Entry * entry_1 = cxi_open_entry(file->entries[0]);
  if(entry_1->experiment_identifier) printf("Experiment Identifier = %s\n", entry_1->experiment_identifier);
  if(entry_1->start_time) printf("Start Time = %s\n", entry_1->start_time);
  
  /* /entry_1/sample_1 */
  if(entry_1->sample_count >= 1){
    CXI_Sample * sample_1 = cxi_open_sample(entry_1->samples[0]);
    if(sample_1->name) printf("Sample name = %s\n", sample_1->name);
  }

  /* /entry_1/instrument_1 */
  if(entry_1->instrument_count >= 1){
    CXI_Instrument * instrument_1 = cxi_open_instrument(entry_1->instruments[0]);
    if(instrument_1->name) printf("Instrument name = %s\n", instrument_1->name);
    
    /* /entry_1/instrument_1/source_1 */
    if(instrument_1->source_count >= 1){
      CXI_Source * source_1 = cxi_open_source(instrument_1->sources[0]);
      if(source_1->energy_valid) printf("Source energy = %g J\n", source_1->energy);
      if(source_1->pulse_width_valid) printf("Source pulse width = %g s\n", source_1->pulse_width);
    }

    /* Read all detectors /entry_1/instrument_1/detector_n */
    for(int n = 0;n< instrument_1->detector_count; n++){
      CXI_Detector * det = cxi_open_detector(instrument_1->detectors[n]);      
      if(det->distance_valid) printf("Detector_%d distance = %g m\n", n+1, det->distance);
      
      if(det->data){
	/* Calculate the image mean */
	CXI_Dataset * dataset = cxi_open_dataset(det->data);
	float * data = malloc(sizeof(float)*cxi_dataset_length(dataset));
	cxi_read_dataset(dataset, data, H5T_NATIVE_FLOAT);
	double sum = 0;
	for(hsize_t i = 0; i < cxi_dataset_length(dataset); i++){
	  sum += data[i];
	}
	sum /= cxi_dataset_length(dataset);
	printf("Detector_%d data mean = %g ADUs/pixel\n", n+1, sum);
      }
    }
  }  
  return 0;
}

