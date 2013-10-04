#include <stdlib.h>
#include <cxi.h>

int main(int argc, char ** argv){
  CXI_File * file = cxi_open_file(argv[1],"r");
  CXI_Entry * entry = cxi_open_entry(file->entries[0]);
  CXI_Instrument * inst = cxi_open_instrument(entry->instruments[0]);
  CXI_Source * source = cxi_open_source(inst->sources[0]);
  CXI_Detector * det = cxi_open_detector(inst->detectors[0]);

  float * image = malloc(sizeof(float)*det->data->size);
  float * slice = malloc(sizeof(float)*det->data->slice_size);
  cxi_read_dataset(det->data,image,H5T_NATIVE_FLOAT);
  cxi_read_dataset_slice(det->data,2,slice,H5T_NATIVE_FLOAT);
  

  printf("%s\n",entry->start_time);
  printf("%s\n",inst->name);
  printf("%g\n",source->energy);
  printf("%g\n",image[0]);
  printf("%g\n",slice[0]);
  return 0;
}
