#include <stdlib.h>
#include <cxi.h>

int main(int argc, char ** argv){
  if(argc < 2){
    printf("Usage: simple <cxi file>\n");
    return 0;
  }

  CXI_File * file = cxi_open_file(argv[1],"r");

  CXI_Entry * entry = NULL;
  if(file && file->entry_count >= 1){
    entry = cxi_open_entry(file->entries[0]);
  }else{
    return -1;
  }

  CXI_Instrument * inst = NULL;
  if(entry && entry->instrument_count >= 1){
    inst = cxi_open_instrument(entry->instruments[0]);
  }else{
    return -1;
  }

  CXI_Source * source = NULL;
  if(inst && inst->source_count >= 1){
    source = cxi_open_source(inst->sources[0]);
  }else{
    return -1;
  }
  
  CXI_Detector * det = NULL;
  CXI_Detector * det2 = NULL;
  if(inst && inst->detector_count >= 2){    
    det = cxi_open_detector(inst->detectors[0]);
    det2 = cxi_open_detector(inst->detectors[1]);
  }else{
    return -1;
  }

  CXI_Sample * sample = NULL;
  if(entry && entry->sample_count >= 1){
    sample = cxi_open_sample(entry->samples[0]);
  }else{
    return -1;
  }

  CXI_Data * data = NULL;
  CXI_Data * data2 = NULL;
  if(entry && entry->data_count >= 2){
    data = cxi_open_data(entry->data[0]);
    data2 = cxi_open_data(entry->data[1]);
  }else{
    return -1;
  }


  CXI_Dataset * data_dataset = cxi_open_dataset(det->data);
  float * image = malloc(sizeof(float)*data_dataset->size);
  float * slice = malloc(sizeof(float)*data_dataset->slice_size);
  if(!image || !slice){
    return  -1;
  }

  cxi_read_dataset(data_dataset,image,H5T_NATIVE_FLOAT);
  cxi_read_dataset_slice(data_dataset,2,slice,H5T_NATIVE_FLOAT);
  

  if(entry->start_time){
    printf("start_time = %s\n",entry->start_time);
  }
  if(inst->name){
    printf("%s\n",inst->name);
  }
  if(source->energy_valid){
    printf("%g\n",source->energy);
  }
  cxi_close_file(file);
  printf("image[0] = %g\n",image[0]);
  printf("slice[0] = %g\n",slice[0]);

  /* check for HDF5 things not properly closed */ 
  int nopen = H5Fget_obj_count(H5F_OBJ_ALL,H5F_OBJ_ALL);
  if(nopen){
    hid_t * obj_list  = malloc(sizeof(hid_t)*nopen);
    H5Fget_obj_ids(H5F_OBJ_ALL, H5F_OBJ_ALL, nopen,obj_list);
    for(int i = 0;i<nopen;i++){
      printf("id %d still open\n", obj_list[i]);
      char buffer[1024];
      if(H5Iget_type(obj_list[i]) == H5I_DATATYPE){	
	continue;
      }
      H5Iget_name(obj_list[i], buffer, 1023);
      printf("%s still open\n",buffer);
    }
    return nopen;
  }

  return 0;
}
