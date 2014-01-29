/* Include the necessary header. */
#include <cxi.h>
#include <stdlib.h>

/* We'll skimp on the error checking for the sake of clarity */
int main(void){
  char * filename = "../data/simple.cxi";
  /* Open the CXI file for reading */
  CXI_File * file = cxi_open_file(filename,"r");

  /* Open /entry_1 */
  CXI_Entry * entry_1 = cxi_open_entry(file->entries[0]);

  /* Open /entry_1/data_1 */
  CXI_Data * data_1 = cxi_open_data(entry_1->data[0]);

  /* Open /entry_1/data_1/data */
  CXI_Dataset * data = cxi_open_dataset(data_1->data);

  /* Allocate space to read the data */
  float * image = malloc(sizeof(float)*cxi_dataset_length(data));

  /* Read the dataset converting to float if necessary */
  cxi_read_dataset(data,image,H5T_NATIVE_FLOAT);

  /* Sum all the values in the image */
  double sum = 0;
  for(hsize_t i = 0;i<cxi_dataset_length(data);i++){
    sum += image[i];
  }
  printf("The sum of all elements of /entry_1/data_1/data is %g\n",sum);
  return 0;
}

