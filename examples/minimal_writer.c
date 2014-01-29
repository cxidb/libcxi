#include <stdlib.h>
#include <string.h>
#include <cxi.h>

int main(int argc, char ** argv){
  char * filename = "simple.cxi";
  if(argc >= 2){
    if(strcmp(argv[1],"-h") == 0){
      printf("Usage: simple_writer [output filename]\n\n");

      printf("By default the output filename is \"simple.cxi\"\n");
      return 0;
    }else{
      filename = argv[1];
    }    
  }
  /* Open the file for writing*/
  CXI_File * file = cxi_open_file(filename,"w");
  if(!file) return -1;
  
  /* Create a bare Entry structure. 
   * It's *necessary* to zero it, so use calloc, *not* malloc. 
   */
  CXI_Entry * entry = calloc(sizeof(CXI_Entry),1);

  /* Write the entry to the file */
  CXI_Entry_Reference * entry_ref = cxi_create_entry(file->handle,entry);
  if(!entry_ref) return -1;
  
  /* Same for Data */
  CXI_Data * data = calloc(sizeof(CXI_Data),1);  
  CXI_Data_Reference * data_ref = cxi_create_data(entry->handle,data);
  if(!data_ref) return -1;

  /* Prepare the Dataset */
  CXI_Dataset * dataset = calloc(sizeof(CXI_Dataset),1);  
  dataset->dimension_count = 1;
  dataset->dimensions = malloc(sizeof(hsize_t));
  dataset->dimensions[0] = 10;
  /* The datatype to be used in the HDF5 file. */
  dataset->data_type = H5T_NATIVE_SHORT;
  /* Create the dataset in the file. */
  CXI_Dataset_Reference * dataset_ref  = cxi_create_dataset(data->handle, dataset, CXI_Data_Type);
  if(!dataset_ref) return -1;
  
  int data_buffer[10] = {1,2,3,4,5,6,7,8,9,10};
  /* Write data to dataset. Specify the datatype of the data in memory. */
  if(cxi_write_dataset(dataset, data_buffer, H5T_NATIVE_INT)) return -1;


  return 0;
}
