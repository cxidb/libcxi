#include <stdlib.h>
#include <string.h>
#include <cxi.h>

int main(int argc, char ** argv){
  char * filename = "typical_raw.cxi";
  if(argc >= 2){
    if(strcmp(argv[1],"-h") == 0){
      printf("Usage: simple_writer [output filename]\n\n");

      printf("By default the output filename is \"typical_raw.cxi\"\n");
      return 0;
    }else{
      filename = argv[1];
    }    
  }
  /* Open the file for writing*/
  CXI_File * file = cxi_open_file(filename,"w");
  if(!file) return -1;
  
  /* Create an Entry structure and fill it. 
   * It's *necessary* to zero it, so use calloc, *not* malloc. 
   */
  CXI_Entry * entry = calloc(sizeof(CXI_Entry),1);
  entry->experiment_identifier = strdup("L730");
  entry->start_time = strdup("2013-01-12T08:00:00+0100");

  /* Write the entry to the file */
  CXI_Entry_Reference * entry_ref = cxi_create_entry(file->handle,entry);
  if(!entry_ref) return -1;
  
  /* Same for Sample */
  CXI_Sample * sample = calloc(sizeof(CXI_Sample),1);  
  sample->name = strdup("Mimivirus");

  CXI_Sample_Reference * sample_ref = cxi_create_sample(entry->handle,sample);
  if(!sample_ref) return -1;

  /* And now the instrument */
  CXI_Instrument * instrument = calloc(sizeof(CXI_Instrument),1);  
  instrument->name = strdup("AMO");
  CXI_Instrument_Reference * instrument_ref = cxi_create_instrument(entry->handle,instrument);
  if(!instrument_ref) return -1;

  CXI_Source * source = calloc(sizeof(CXI_Source),1);  
  source->energy = 2.8893e-16;
  source->energy_valid = 1;
  source->pulse_width = 7e-14;
  source->pulse_width_valid = 1;
  CXI_Source_Reference * source_ref = cxi_create_source(instrument->handle,source);
  if(!source_ref) return -1;

  /* Write the first detector */
  CXI_Detector * det = calloc(sizeof(CXI_Detector),1);  
  det->distance = 0.15;
  det->distance_valid = 1;
  CXI_Detector_Reference * detector_ref = cxi_create_detector(instrument->handle,det);
  if(!detector_ref) return -1;

  /* Prepare the Dataset */
  CXI_Dataset * dataset = calloc(sizeof(CXI_Dataset),1);  
  dataset->dimension_count = 1;
  dataset->dimensions = malloc(sizeof(hsize_t));
  dataset->dimensions[0] = 10;
  /* The datatype to be used in the HDF5 file. */
  dataset->data_type = H5T_NATIVE_SHORT;
  /* Create the dataset in the file. */
  CXI_Dataset_Reference * dataset_ref  = cxi_create_dataset(det->handle, dataset, CXI_Data_Type);
  if(!dataset_ref) return -1;
  
  int data_buffer[10] = {1,2,3,4,5,6,7,8,9,10};
  /* Write data to dataset. Specify the datatype of the data in memory. */
  if(cxi_write_dataset(dataset, data_buffer, H5T_NATIVE_INT)) return -1;
  /* Note that you can only create the link after cxi_create_dataset */
  cxi_create_data_link(entry,dataset);

  /* Write the second detector */
  det = calloc(sizeof(CXI_Detector),1);  
  det->distance = 0.65;
  det->distance_valid = 1;
  detector_ref = cxi_create_detector(instrument->handle,det);
  if(!detector_ref) return -1;

  /* Prepare the Dataset */
  dataset = calloc(sizeof(CXI_Dataset),1);  
  dataset->dimension_count = 1;
  dataset->dimensions = malloc(sizeof(hsize_t));
  dataset->dimensions[0] = 10;
  /* The datatype to be used in the HDF5 file. */
  dataset->data_type = H5T_NATIVE_SHORT;
  /* Create the dataset in the file. */
  dataset_ref  = cxi_create_dataset(det->handle, dataset, CXI_Data_Type);
  if(!dataset_ref) return -1;
  
  int data2_buffer[10] = {11,12,13,14,15,16,17,18,19,20};
  /* Write data to dataset. Specify the datatype of the data in memory. */
  if(cxi_write_dataset(dataset, data2_buffer, H5T_NATIVE_INT)) return -1;
  cxi_create_data_link(entry,dataset);

  return 0;
}
