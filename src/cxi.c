#include <stdlib.h>
#include <string.h>
#include <cxi.h>



static int find_max_suffix(hid_t loc, char *basename){
  int n;
  for(n = 1;;n++){
    char buffer[1024];
    sprintf(buffer,"%s_%d",basename,n);
    if(!H5Lexists(loc,buffer,H5P_DEFAULT)){
      break;
    }
  }
  return n-1;
}


static int is_scalar(hid_t dataset){
   hid_t s = H5Dget_space(dataset);
   int dims = H5Sget_simple_extent_ndims(s);
   H5Sclose(s);
   if(dims == 0){
     return 1;
   }else{
     return 0;
   }
}

static int array_total_size(hid_t dataset){
   hid_t s = H5Dget_space(dataset);
   int ndims = H5Sget_simple_extent_ndims(s);
   if(ndims == 0){
     return -1;
   }
   hsize_t * dims = malloc(sizeof(hsize_t)*ndims);
   H5Sget_simple_extent_dims(s,dims,NULL);   
   H5Sclose(s);
   int size = 1;
   for(int i = 0;i<ndims;i++){
     size *= dims[i];
   }
   return size;
}

static void try_copy_string(hid_t loc, char * name, char ** dest){
  if(H5Lexists(loc,name,H5P_DEFAULT)){
    hid_t ds = H5Dopen(loc,name,H5P_DEFAULT);
    hid_t t = H5Dget_type(ds);
    if(H5Tget_class(t) == H5T_STRING){
      *dest = malloc(sizeof(char)*H5Tget_size(t));
      H5Dread(ds,t,H5S_ALL,H5S_ALL,H5P_DEFAULT,*dest);
    }
    H5Tclose(t);
    H5Dclose(ds);
  }  
}

static void try_copy_float(hid_t loc, char * name, double * dest){
  if(H5Lexists(loc,name,H5P_DEFAULT)){
    hid_t ds = H5Dopen(loc,name,H5P_DEFAULT);
    hid_t t = H5Dget_type(ds);
    hid_t s = H5Dget_space(ds);
    if(H5Tget_class(t) == H5T_FLOAT && is_scalar(ds)){
      H5Dread(ds,H5T_NATIVE_DOUBLE,H5S_ALL,H5S_ALL,H5P_DEFAULT,dest);
    }
    H5Tclose(t);
    H5Sclose(s);
    H5Dclose(ds);
  }  
}

static void try_copy_float_array(hid_t loc, char * name, double * dest, int size){
  if(H5Lexists(loc,name,H5P_DEFAULT)){
    hid_t ds = H5Dopen(loc,name,H5P_DEFAULT);
    hid_t t = H5Dget_type(ds);
    hid_t s = H5Dget_space(ds);
    if(H5Tget_class(t) == H5T_FLOAT &&  array_total_size(ds) == size){
      H5Dread(ds,H5T_NATIVE_DOUBLE,H5S_ALL,H5S_ALL,H5P_DEFAULT,dest);
    }
    H5Tclose(t);
    H5Sclose(s);
    H5Dclose(ds);
  }  
}

static void try_copy_dataset(hid_t loc, char * name, CXI_Dataset ** dest){
  if(!H5Lexists(loc,name,H5P_DEFAULT)){
    return;
  }
  *dest = calloc(sizeof(CXI_Dataset),1);
  CXI_Dataset * ds = *dest;
  ds->handle = H5Dopen(loc,name,H5P_DEFAULT);

  hid_t s = H5Dget_space(ds->handle);
  ds->dimension_count = H5Sget_simple_extent_ndims(s);
  ds->dimensions = calloc(sizeof(hsize_t),ds->dimension_count);
  H5Sget_simple_extent_dims(s,ds->dimensions,NULL);     
  ds->size = 1;
  for(int i = 0;i<ds->dimension_count;i++){
    ds->size *= ds->dimensions[i];
  }
  ds->slice_size = ds->size/ds->dimensions[0];


  ds->datatype = H5Dget_type(ds->handle);
}

CXI_File * cxi_open_file(const char * filename, const char * mode){
  CXI_File * file = malloc(sizeof(CXI_File));
  if(!file){
    return NULL;
  }
  if(strcmp(mode,"r") == 0){
    file->handle = H5Fopen(filename, H5F_ACC_RDONLY,H5P_DEFAULT);
    file->filename = malloc(sizeof(char)*(strlen(filename)+1));
    strcpy(file->filename,filename);
    int n = find_max_suffix(file->handle, "entry");
    file->entry_count = n;
    file->entries = calloc(sizeof(CXI_Entry_Reference *),n);
    char buffer[1024];
    for(int i = 0;i<n;i++){
      file->entries[i] = calloc(sizeof(CXI_Entry_Reference),1);
      sprintf(buffer,"entry_%d",i+1);
      file->entries[i]->parent_handle = file->handle;
      file->entries[i]->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
      strcpy(file->entries[i]->group_name,buffer);      
    }
    return file;
  }else{
    free(file);
    return NULL;
  }    
}
int cxi_close(CXI_File * file){
  H5Fclose(file->handle);
  free(file->filename);
  free(file);
  return 0;
}

CXI_Entry * cxi_open_entry(CXI_Entry_Reference * ref){
  if(!ref){
    return NULL;
  }
  CXI_Entry * entry = calloc(sizeof(CXI_Entry),1);
  if(!entry){
    return NULL;
  }

  entry->handle = H5Gopen(ref->parent_handle,ref->group_name,H5P_DEFAULT);
  if(entry->handle < 0){
    free(entry);
    return NULL;
  }

  char buffer[1024];
  int n;
  /* Search for Data groups */
  n = find_max_suffix(entry->handle, "data");
  entry->data_count = n;
  entry->data = calloc(sizeof(CXI_Data_Reference *),n);
  for(int i = 0;i<n;i++){
    entry->data[i] = calloc(sizeof(CXI_Data_Reference),1);
    sprintf(buffer,"data_%d",i+1);
    entry->data[i]->parent_handle = entry->handle;
    entry->data[i]->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(entry->data[i]->group_name,buffer);      
  }


  /* Search for Image groups */
  n = find_max_suffix(entry->handle, "image");
  entry->image_count = n;
  entry->images = calloc(sizeof(CXI_Image *),n);
  for(int i = 0;i<n;i++){
    entry->images[i] = calloc(sizeof(CXI_Image),1);
    sprintf(buffer,"image_%d",i+1);
    entry->images[i]->parent_handle = entry->handle;
    entry->images[i]->handle = -1;
    entry->images[i]->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(entry->images[i]->group_name,buffer);      
  }

  /* Search for Instrument groups */
  n = find_max_suffix(entry->handle, "instrument");
  entry->instrument_count = n;
  entry->instruments = calloc(sizeof(CXI_Instrument *),n);
  for(int i = 0;i<n;i++){
    entry->instruments[i] = calloc(sizeof(CXI_Instrument),1);
    sprintf(buffer,"instrument_%d",i+1);
    entry->instruments[i]->parent_handle = entry->handle;
    entry->instruments[i]->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(entry->instruments[i]->group_name,buffer);      
  }

  /* Search for Sample groups */
  n = find_max_suffix(entry->handle, "sample");
  entry->sample_count = n;
  entry->samples = calloc(sizeof(CXI_Sample *),n);
  for(int i = 0;i<n;i++){
    entry->samples[i] = calloc(sizeof(CXI_Sample),1);
    sprintf(buffer,"sample_%d",i+1);
    entry->samples[i]->parent_handle = entry->handle;
    entry->samples[i]->handle = -1;
    entry->samples[i]->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(entry->samples[i]->group_name,buffer);      
  }

  /* Search for Instrument groups */
  n = find_max_suffix(entry->handle, "instrument");
  entry->instrument_count = n;
  entry->instruments = calloc(sizeof(CXI_Instrument_Reference *),n);
  for(int i = 0;i<n;i++){
    entry->instruments[i] = calloc(sizeof(CXI_Instrument_Reference),1);
    sprintf(buffer,"instrument_%d",i+1);
    entry->instruments[i]->parent_handle = entry->handle;
    entry->instruments[i]->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(entry->instruments[i]->group_name,buffer);      
  }

  /* Now lets try to fill in whatever we can */
  try_copy_string(entry->handle, "end_time",&entry->end_time);
  try_copy_string(entry->handle, "experiment_identifier",&entry->experiment_identifier);
  try_copy_string(entry->handle, "experiment_description",&entry->experiment_description);
  try_copy_string(entry->handle, "program_name",&entry->program_name);
  try_copy_string(entry->handle, "start_time",&entry->start_time);
  try_copy_string(entry->handle, "title",&entry->title);
  return entry;
}

CXI_Instrument * cxi_open_instrument(CXI_Instrument_Reference * ref){
  char buffer[1024];
  if(!ref){
    return NULL;
  }
  CXI_Instrument * instrument = calloc(sizeof(CXI_Instrument),1);
  if(!instrument){
    return NULL;
  }

  instrument->handle = H5Gopen(ref->parent_handle,ref->group_name,H5P_DEFAULT);
  if(instrument->handle < 0){
    free(instrument);
    return NULL;
  }
  int n;
  /* Search for Attenuator groups */
  n = find_max_suffix(instrument->handle, "attenuator");
  instrument->attenuator_count = n;
  instrument->attenuators = calloc(sizeof(CXI_Attenuator *),n);
  for(int i = 0;i<n;i++){
    instrument->attenuators[i] = calloc(sizeof(CXI_Attenuator),1);
    sprintf(buffer,"attenuator_%d",i+1);
    instrument->attenuators[i]->parent_handle = instrument->handle;
    instrument->attenuators[i]->handle = -1;
    instrument->attenuators[i]->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(instrument->attenuators[i]->group_name,buffer);      
  }

  /* Search for Detector groups */
  n = find_max_suffix(instrument->handle, "detector");
  instrument->detector_count = n;
  instrument->detectors = calloc(sizeof(CXI_Detector_Reference *),n);
  for(int i = 0;i<n;i++){
    instrument->detectors[i] = calloc(sizeof(CXI_Detector_Reference),1);
    sprintf(buffer,"detector_%d",i+1);
    instrument->detectors[i]->parent_handle = instrument->handle;
    instrument->detectors[i]->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(instrument->detectors[i]->group_name,buffer);      
  }


  /* Search for Monochromator groups */
  n = find_max_suffix(instrument->handle, "monochromator");
  instrument->monochromator_count = n;
  instrument->monochromators = calloc(sizeof(CXI_Monochromator *),n);
  for(int i = 0;i<n;i++){
    instrument->monochromators[i] = calloc(sizeof(CXI_Monochromator),1);
    sprintf(buffer,"monochromator_%d",i+1);
    instrument->monochromators[i]->parent_handle = instrument->handle;
    instrument->monochromators[i]->handle = -1;
    instrument->monochromators[i]->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(instrument->monochromators[i]->group_name,buffer);      
  }

  /* Search for Source groups */
  n = find_max_suffix(instrument->handle, "source");
  instrument->source_count = n;
  instrument->sources = calloc(sizeof(CXI_Source *),n);
  for(int i = 0;i<n;i++){
    instrument->sources[i] = calloc(sizeof(CXI_Source),1);
    sprintf(buffer,"source_%d",i+1);
    instrument->sources[i]->parent_handle = instrument->handle;
    instrument->sources[i]->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(instrument->sources[i]->group_name,buffer);      
  }


  /* Now lets try to fill in whatever we can */
  try_copy_string(instrument->handle, "name",&instrument->name);
  return instrument;
}

CXI_Source * cxi_open_source(CXI_Source_Reference * ref){
  char buffer[1024];
  if(!ref){
    return NULL;
  }
  CXI_Source * source = calloc(sizeof(CXI_Source),1);
  if(!source){
    return NULL;
  }
  source->handle = H5Gopen(ref->parent_handle,ref->group_name,H5P_DEFAULT);
  if(source->handle < 0){
    return NULL;
  }

  try_copy_string(source->handle, "name",&source->name);
  try_copy_float(source->handle, "energy",&source->energy);
  try_copy_float(source->handle, "pulse_energy",&source->pulse_energy);
  try_copy_float(source->handle, "pulse_width",&source->pulse_width);
  return source;
}

CXI_Detector * cxi_open_detector(CXI_Detector_Reference * ref){
  char buffer[1024];
  if(!ref){
    return NULL;
  }
  CXI_Detector * detector = calloc(sizeof(CXI_Detector),1);
  if(!detector){
    return NULL;
  }

  detector->handle = H5Gopen(ref->parent_handle,ref->group_name,H5P_DEFAULT);
  if(detector->handle < 0){
    free(detector);
    return NULL;
  }

  int n;
  /* Search for Geometry groups */
  n = find_max_suffix(detector->handle, "geometry");
  detector->geometry_count = n;
  detector->geometries = calloc(sizeof(CXI_Geometry_Reference *),n);
  for(int i = 0;i<n;i++){
    detector->geometries[i] = calloc(sizeof(CXI_Geometry_Reference),1);
    sprintf(buffer,"geometry_%d",i+1);
    detector->geometries[i]->parent_handle = detector->handle;
    detector->geometries[i]->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(detector->geometries[i]->group_name,buffer);      
  }

  try_copy_float_array(detector->handle, "basis_vectors",(double *)detector->basis_vectors,9);
  try_copy_float_array(detector->handle, "corner_position",(double *)detector->corner_position,3);
  try_copy_float(detector->handle, "counts_per_joule",&detector->counts_per_joule);
  try_copy_float(detector->handle, "data_sum",&detector->data_sum);
  try_copy_string(detector->handle, "description",&detector->description);
  try_copy_float(detector->handle, "distance",&detector->distance);
  try_copy_float(detector->handle, "x_pixel_size",&detector->x_pixel_size);
  try_copy_float(detector->handle, "y_pixel_size",&detector->y_pixel_size);

  try_copy_dataset(detector->handle, "data",&detector->data);
  try_copy_dataset(detector->handle, "data_dark",&detector->data_dark);
  try_copy_dataset(detector->handle, "data_white",&detector->data_white);
  try_copy_dataset(detector->handle, "data_error",&detector->data_error);
  try_copy_dataset(detector->handle, "mask",&detector->mask);
  return detector;
}

int cxi_read_dataset(CXI_Dataset * dataset, void * data, hid_t datatype){
  if(!dataset){
    return -1;
  }
  if(!data){
    return -1;
  }
  H5Dread(dataset->handle,datatype,H5S_ALL,H5S_ALL,H5P_DEFAULT,data);      
  return 0;
}

int cxi_read_dataset_slice(CXI_Dataset * dataset, int slice, void * data, hid_t datatype){
  if(!dataset){
    return -1;
  }
  if(!data){
    return -1;
  }
  if(slice < 0 || slice >= dataset->dimensions[0]){
    return -1;
  }

  hid_t s = H5Dget_space(dataset->handle);
  if(s < 0){
    return -1;
  }
  hsize_t *start;
  start = malloc(sizeof(hsize_t)*dataset->dimension_count);
  hsize_t *count;
  count = malloc(sizeof(hsize_t)*dataset->dimension_count);
  for(int i =0;i<dataset->dimension_count;i++){
    start[i] = 0;
    count[i] = dataset->dimensions[i];
  }

  start[0] = slice;
  count[0] = 1;
  hid_t memspace = H5Screate_simple (dataset->dimension_count, count, NULL);
  
  H5Sselect_hyperslab(s, H5S_SELECT_SET, start, NULL, count, NULL);
  H5Dread(dataset->handle,datatype,memspace,s,H5P_DEFAULT,data);      
  H5Sclose(s);
  free(start);
  free(count);

  return 0;
}




