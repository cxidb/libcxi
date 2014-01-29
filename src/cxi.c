#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cxi.h"
#include <stdarg.h>


#if 1 || defined __CXI_DEBUG 
#define  cxi_debug(...) _cxi_debug(__FILE__,__LINE__,__VA_ARGS__)
#else  
#define  cxi_debug(...) 
#endif 


static int CXI_VERSION = 130;

static void _cxi_debug(char * file, int line, char *format, ...){
  va_list ap;
  va_start(ap,format);
  fprintf(stderr, "libcxi debug: ");
  vfprintf(stderr, format, ap);
  fprintf(stderr, " in %s:%d\n",file,line);
  va_end(ap);
}

static void cxi_close_data(CXI_Data_Reference * ref);
static void cxi_close_entry(CXI_Entry_Reference * ref);
static void cxi_close_instrument(CXI_Instrument_Reference * ref);
static void cxi_close_source(CXI_Source_Reference * ref);
static void cxi_close_detector(CXI_Detector_Reference * ref);
static void cxi_close_attenuator(CXI_Attenuator_Reference * ref);
static void cxi_close_monochromator(CXI_Monochromator_Reference * ref);
static void cxi_close_image(CXI_Image_Reference * ref);
static void cxi_close_sample(CXI_Sample_Reference * ref);
static void cxi_close_dataset(CXI_Dataset_Reference * data);

static int follows_iso8601(char * date){
  /* We'll only support dates with 4 digit years */
  for(int i = 0;i<4;i++){
    if(!isdigit(date[i])){
      return 0;
    }
  }
  if(date[4] != '-'){
    return 0;
  }
  /* check month*/
  for(int i = 5;i<7;i++){
    if(!isdigit(date[i])){
      return 0;
    }
  }
  if(date[7] != '-'){
    return 0;
  }
  /* check day*/
  for(int i = 8;i<10;i++){
    if(!isdigit(date[i])){
      return 0;
    }
  }
  if(date[10] != 'T'){
    return 0;
  }
  /* check hour*/
  for(int i = 11;i<13;i++){
    if(!isdigit(date[i])){
      return 0;
    }
  }
  if(date[13] != ':'){
    return 0;
  }
  /* check minute*/
  for(int i = 14;i<16;i++){
    if(!isdigit(date[i])){
      return 0;
    }
  }
  if(date[16] != ':'){
    return 0;
  }
  /* check second */
  for(int i = 17;i<19;i++){
    if(!isdigit(date[i])){
      return 0;
    }
  }
  if(date[19] != '+' && date[19] != '-'){
    return 0; 
  }
  /* check timezone */
  for(int i = 20;i<24;i++){
    if(!isdigit(date[i])){
      return 0;
    }
  }
  return 1;
}


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

static int try_read_string(hid_t loc, char * name, char ** dest){
  if(H5Lexists(loc,name,H5P_DEFAULT)){
    hid_t ds = H5Dopen(loc,name,H5P_DEFAULT);
    hid_t t = H5Dget_type(ds);
    if(H5Tget_class(t) == H5T_STRING){
      *dest = malloc(sizeof(char)*H5Tget_size(t));
      H5Dread(ds,t,H5S_ALL,H5S_ALL,H5P_DEFAULT,*dest);
    }
    H5Tclose(t);
    H5Dclose(ds);
    return 1;
  }  
  return 0;
}

static int try_write_string(hid_t loc, char * name, char * values){
  hid_t space = H5Screate(H5S_SCALAR);
  if(space < 0) return space;
  hid_t datatype = H5Tcopy(H5T_C_S1);
  if(datatype < 0) return space;
  H5Tset_size(datatype, strlen(values));
  hid_t ds = H5Dcreate(loc,name,datatype,space,H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  if(ds < 0) return ds;
  herr_t status = H5Sclose(space);
  if(status < 0) return status;    
  status = H5Dwrite(ds,datatype,H5S_ALL,H5S_ALL,H5P_DEFAULT,values);
  if(status < 0) return status;    
  return H5Dclose(ds);
}

static int try_read_float(hid_t loc, char * name, double * dest){
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
    return 1;
  }  
  return 0;
}

static int try_write_float(hid_t loc, char * name, double value){
  hid_t space = H5Screate(H5S_SCALAR);
  if(space < 0) return space;
  hid_t ds = H5Dcreate(loc,name,H5T_NATIVE_FLOAT,space,H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  if(ds < 0) return ds;
  herr_t status = H5Sclose(space);
  if(status < 0) return status;    
  status = H5Dwrite(ds,H5T_NATIVE_DOUBLE,H5S_ALL,H5S_ALL,H5P_DEFAULT,&value);
  if(status < 0) return status;    
  return H5Dclose(ds);
}

static int try_read_int(hid_t loc, char * name, int * dest){
  if(H5Lexists(loc,name,H5P_DEFAULT)){
    hid_t ds = H5Dopen(loc,name,H5P_DEFAULT);
    hid_t t = H5Dget_type(ds);
    hid_t s = H5Dget_space(ds);
    if(H5Tget_class(t) == H5T_INTEGER && is_scalar(ds)){
      H5Dread(ds,H5T_NATIVE_INT32,H5S_ALL,H5S_ALL,H5P_DEFAULT,dest);
    }
    H5Tclose(t);
    H5Sclose(s);
    H5Dclose(ds);
    return 1;
  }  
  return 0;
}

static int try_read_float_array(hid_t loc, char * name, double * dest, int size){
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
    return 1;
  }  
  return 0;
}

static int try_write_float_array(hid_t loc, char * name, double * values, int ndims, hsize_t * dims){
  hid_t space = H5Screate_simple(ndims,dims,NULL);
  if(space < 0) return space;
  hid_t ds = H5Dcreate(loc,name,H5T_NATIVE_FLOAT,space,H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  if(ds < 0) return ds;
  herr_t status = H5Sclose(space);
  if(status < 0) return status;    
  status = H5Dwrite(ds,H5T_NATIVE_DOUBLE,H5S_ALL,H5S_ALL,H5P_DEFAULT,values);
  if(status < 0) return status;    
  return H5Dclose(ds);
}

static int try_write_float_1D_array(hid_t loc, char * name, double * values, int dim){
  hsize_t dims[1];
  dims[0] = dim;
  return try_write_float_array(loc, name, values, 1, dims);
}

static int try_write_float_2D_array(hid_t loc, char * name, double * values, int slow_dim, int fast_dim){
  hsize_t dims[2];
  dims[0] = slow_dim;
  dims[1] = fast_dim;
  return try_write_float_array(loc, name, values, 2, dims);
}

static int try_write_float_3D_array(hid_t loc, char * name, double * values, int slow_dim, int middle_dim, int fast_dim){
  hsize_t dims[3];
  dims[0] = slow_dim;
  dims[1] = middle_dim;
  dims[2] = fast_dim;
  return try_write_float_array(loc, name, values, 3, dims);
}



CXI_Data * cxi_open_data(CXI_Data_Reference * ref){
  cxi_debug("opening data");
  char buffer[1024];
  if(!ref){
    return NULL;
  }
  CXI_Data * data = calloc(sizeof(CXI_Data),1);
  if(!data){
    return NULL;
  }

  data->handle = H5Gopen(ref->parent_handle,ref->group_name,H5P_DEFAULT);
  if(data->handle < 0){
    free(data);
    return NULL;
  }
  ref->data = data;  
  if(H5Lexists(data->handle,"data",H5P_DEFAULT)){
    data->data = calloc(sizeof(CXI_Dataset_Reference),1);
    data->data->parent_handle = data->handle;
    data->data->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(data->data->group_name,"data");          
  }

  if(H5Lexists(data->handle,"errors",H5P_DEFAULT)){
    data->errors = calloc(sizeof(CXI_Dataset_Reference),1);
    data->errors->parent_handle = data->handle;
    data->errors->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(data->errors->group_name,"errors");          
  }
  return data;
}

static void cxi_close_data(CXI_Data_Reference * ref){
  cxi_debug("closing data");
  CXI_Data * data = ref->data;
  if(data){
    cxi_close_dataset(data->data);
    H5Gclose(data->handle);
    free(data);
  }
  free(ref);
}


CXI_File * cxi_open_file(const char * filename, const char * mode){
  cxi_debug("opening file");
  CXI_File * file = malloc(sizeof(CXI_File));
  if(!file){
    return NULL;
  }
  if(strcmp(mode,"r") == 0){
    file->handle = H5Fopen(filename, H5F_ACC_RDONLY,H5P_DEFAULT);
    if(file->handle < 0){
      free(file);
      return NULL;
    }
    file->filename = malloc(sizeof(char)*(strlen(filename)+1));
    strcpy(file->filename,filename);
    /* Read existing entries */
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
    /* Read the CXI verion */
    file->cxi_version = -1;
    try_read_int(file->handle, "cxi_version",&file->cxi_version);
    if(file->cxi_version < 0){
      /* Warning: Could not read CXI version */
    }else if(file->cxi_version >= CXI_VERSION){
      /* Warning: CXI version of the file is higher than from libcxi */
    }
    return file;    
  }else if(strcmp(mode,"w") == 0){
    file->handle = H5Fcreate(filename,H5F_ACC_TRUNC,H5P_DEFAULT,H5P_DEFAULT);
    file->filename = malloc(sizeof(char)*(strlen(filename)+1));
    strcpy(file->filename,filename);
    hsize_t dims[1] = {1};
    hid_t dataspace = H5Screate_simple(1, dims, dims);
    hid_t dataset = H5Dcreate(file->handle, "cxi_version", H5T_NATIVE_INT, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &CXI_VERSION);
    H5Dclose(dataset);
    H5Sclose(dataspace);
    return file;
  }else{
    free(file);
    return NULL;
  }    
}

int cxi_close_file(CXI_File * file){
  cxi_debug("closing file");
  for(int i = 0;i<file->entry_count;i++){
    cxi_close_entry(file->entries[i]);
  }
  H5Fclose(file->handle);
  free(file->entries);
  free(file->filename);
  free(file);
  return 0;
}

CXI_Entry * cxi_open_entry(CXI_Entry_Reference * ref){
  cxi_debug("opening entry");
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
  ref->entry = entry;

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
  entry->images = calloc(sizeof(CXI_Image_Reference *),n);
  for(int i = 0;i<n;i++){
    entry->images[i] = calloc(sizeof(CXI_Image_Reference),1);
    sprintf(buffer,"image_%d",i+1);
    entry->images[i]->parent_handle = entry->handle;
    entry->images[i]->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(entry->images[i]->group_name,buffer);      
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

  /* Search for Sample groups */
  n = find_max_suffix(entry->handle, "sample");
  entry->sample_count = n;
  entry->samples = calloc(sizeof(CXI_Sample_Reference *),n);
  for(int i = 0;i<n;i++){
    entry->samples[i] = calloc(sizeof(CXI_Sample_Reference),1);
    sprintf(buffer,"sample_%d",i+1);
    entry->samples[i]->parent_handle = entry->handle;
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
  try_read_string(entry->handle, "end_time",&entry->end_time);
  try_read_string(entry->handle, "experiment_identifier",&entry->experiment_identifier);
  try_read_string(entry->handle, "experiment_description",&entry->experiment_description);
  try_read_string(entry->handle, "program_name",&entry->program_name);
  try_read_string(entry->handle, "start_time",&entry->start_time);
  try_read_string(entry->handle, "title",&entry->title);
  return entry;
}


static void cxi_close_entry(CXI_Entry_Reference * ref){
  cxi_debug("closing entry");
  CXI_Entry * entry = ref->entry;
  for(int i = 0;i<entry->data_count;i++){
    cxi_close_data(entry->data[i]);
  }
  free(entry->data);
  for(int i = 0;i<entry->image_count;i++){
    cxi_close_image(entry->images[i]);
  }
  free(entry->images);
  for(int i = 0;i<entry->instrument_count;i++){
    cxi_close_instrument(entry->instruments[i]);
  }
  free(entry->instruments);
  for(int i = 0;i<entry->sample_count;i++){
    cxi_close_sample(entry->samples[i]);
  }
  free(entry->samples);
  H5Gclose(entry->handle);
  free(entry);
  free(ref->group_name);
  free(ref);  
}



CXI_Instrument * cxi_open_instrument(CXI_Instrument_Reference * ref){
  cxi_debug("opening instrument");
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
  ref->instrument = instrument;
  int n;
  /* Search for Attenuator groups */
  n = find_max_suffix(instrument->handle, "attenuator");
  instrument->attenuator_count = n;
  instrument->attenuators = calloc(sizeof(CXI_Attenuator *),n);
  for(int i = 0;i<n;i++){
    instrument->attenuators[i] = calloc(sizeof(CXI_Attenuator),1);
    sprintf(buffer,"attenuator_%d",i+1);
    instrument->attenuators[i]->parent_handle = instrument->handle;
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
  try_read_string(instrument->handle, "name",&instrument->name);
  return instrument;
}

static void cxi_close_instrument(CXI_Instrument_Reference * ref){
  cxi_debug("closing instrument");
  CXI_Instrument * instrument = ref->instrument;
  if(instrument){
    for(int i = 0;i<instrument->detector_count;i++){
      cxi_close_detector(instrument->detectors[i]);
    }
    free(instrument->detectors);
    for(int i = 0;i<instrument->attenuator_count;i++){
      cxi_close_attenuator(instrument->attenuators[i]);
    }
    free(instrument->attenuators);
    for(int i = 0;i<instrument->monochromator_count;i++){
      cxi_close_monochromator(instrument->monochromators[i]);
    }
    free(instrument->monochromators);
    for(int i = 0;i<instrument->source_count;i++){
      cxi_close_source(instrument->sources[i]);
    }
    free(instrument->sources);

    H5Gclose(instrument->handle);
    free(instrument);
  }
  free(ref->group_name);
  free(ref);  
}


CXI_Source * cxi_open_source(CXI_Source_Reference * ref){
  cxi_debug("opening source");
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
  ref->source = source;

  try_read_string(source->handle, "name",&source->name);
  source->energy_valid = try_read_float(source->handle, "energy",&source->energy);
  source->pulse_energy_valid = try_read_float(source->handle, "pulse_energy",&source->pulse_energy);
  source->pulse_width_valid = try_read_float(source->handle, "pulse_width",&source->pulse_width);

  return source;
}

static void cxi_close_source(CXI_Source_Reference * ref){
  cxi_debug("closing source");
  CXI_Source * source = ref->source;
  if(source){
    free(source->name);
    H5Gclose(source->handle);
    free(source);
  }
  free(ref->group_name);
  free(ref);
}


CXI_Detector * cxi_open_detector(CXI_Detector_Reference * ref){
  cxi_debug("opening detector");
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
  ref->detector = detector;

  int n;
  /* Search for Geometry groups */
  n = find_max_suffix(detector->handle, "geometry");
  if(n > 1){
    cxi_debug("Warning: Opened detector with multiple geometries");
  }
  for(int i = 0;i<n;i++){
    detector->geometry = calloc(sizeof(CXI_Geometry_Reference),1);
    sprintf(buffer,"geometry_%d",i+1);
    detector->geometry->parent_handle = detector->handle;
    detector->geometry->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(detector->geometry->group_name,buffer);      
  }


  detector->corner_position_valid = try_read_float_array(detector->handle, 
							 "corner_position",
							 (double *)detector->corner_position,3);
  detector->counts_per_joule_valid = try_read_float(detector->handle,
						    "counts_per_joule",
						    &detector->counts_per_joule);
  detector->data_sum_valid = try_read_float(detector->handle, "data_sum",
					    &detector->data_sum);
  try_read_string(detector->handle, "description",&detector->description);
  detector->distance_valid = try_read_float(detector->handle, 
					    "distance",&detector->distance);
  detector->x_pixel_size_valid = try_read_float(detector->handle,
						"x_pixel_size",
						&detector->x_pixel_size);
  detector->y_pixel_size_valid = try_read_float(detector->handle,
						"y_pixel_size",
						&detector->y_pixel_size);
  if(!detector->x_pixel_size_valid){
    /* couldn't read pixel size, set default */
    detector->x_pixel_size = 1;
  }
  if(!detector->y_pixel_size_valid){
    detector->y_pixel_size = 1;
  }

  detector->basis_vectors_valid = try_read_float_array(detector->handle,
						       "basis_vectors",
						       (double *)detector->basis_vectors,6);
  if(!detector->basis_vectors_valid){
    /* couldn't read basis_vectors, set default */
    detector->basis_vectors[0][0] = 0;
    detector->basis_vectors[0][1] = -detector->y_pixel_size;
    detector->basis_vectors[0][2] = 0;
    detector->basis_vectors[1][0] = -detector->x_pixel_size;
    detector->basis_vectors[1][1] = 0;
    detector->basis_vectors[1][2] = 0;        
  }

  if(H5Lexists(detector->handle,"data",H5P_DEFAULT)){
    detector->data = calloc(sizeof(CXI_Dataset_Reference),1);
    detector->data->parent_handle = detector->handle;
    detector->data->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(detector->data->group_name,"data");          
  }
  if(H5Lexists(detector->handle,"data_dark",H5P_DEFAULT)){
    detector->data_dark = calloc(sizeof(CXI_Dataset_Reference),1);
    detector->data_dark->parent_handle = detector->handle;
    detector->data_dark->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(detector->data_dark->group_name,"data_dark");          
  }

  if(H5Lexists(detector->handle,"data_white",H5P_DEFAULT)){
    detector->data_white = calloc(sizeof(CXI_Dataset_Reference),1);
    detector->data_white->parent_handle = detector->handle;
    detector->data_white->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(detector->data_white->group_name,"data_white");          
  }

  if(H5Lexists(detector->handle,"data_error",H5P_DEFAULT)){
    detector->data_error = calloc(sizeof(CXI_Dataset_Reference),1);
    detector->data_error->parent_handle = detector->handle;
    detector->data_error->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(detector->data_error->group_name,"data_error");          
  }


  if(H5Lexists(detector->handle,"mask",H5P_DEFAULT)){
    detector->mask = calloc(sizeof(CXI_Dataset_Reference),1);
    detector->mask->parent_handle = detector->handle;
    detector->mask->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(detector->mask->group_name,"mask");      
    
  }

  return detector;
}

static void cxi_close_detector(CXI_Detector_Reference * ref){
  cxi_debug("closing detector");
  CXI_Detector * detector = ref->detector;
  if(detector){
    cxi_close_dataset(detector->data);
    cxi_close_dataset(detector->data_white);
    cxi_close_dataset(detector->data_dark);
    cxi_close_dataset(detector->data_error);
    cxi_close_dataset(detector->mask);
    H5Gclose(detector->handle);
    free(detector);
  }
  free(ref->group_name);
  free(ref);
}

CXI_Dataset * cxi_open_dataset(CXI_Dataset_Reference * ref){
  cxi_debug("opening dataset");
  if(!ref){
    return NULL;
  }


  cxi_debug("opening dataset");
  CXI_Dataset * dataset = calloc(sizeof(CXI_Dataset),1);
  if(!dataset){
    return NULL;
  }
  dataset->handle = H5Dopen(ref->parent_handle,ref->group_name,H5P_DEFAULT);

  hid_t s = H5Dget_space(dataset->handle);
  dataset->dimension_count = H5Sget_simple_extent_ndims(s);
  dataset->dimensions = calloc(sizeof(hsize_t),dataset->dimension_count);
  H5Sget_simple_extent_dims(s,dataset->dimensions,NULL);     
  dataset->data_type = H5Dget_type(dataset->handle);
  ref->dataset = dataset;
  return dataset;
}

static void cxi_close_dataset(CXI_Dataset_Reference * ref){
  if(!ref){
    return;
  }
  cxi_debug("closing dataset");
  CXI_Dataset * dataset = ref->dataset;
  if(dataset){
    H5Dclose(dataset->handle);
    H5Tclose(dataset->data_type);
    free(dataset->dimensions);
    free(dataset);
  }
  free(ref->group_name);
  free(ref);
}


CXI_Attenuator * cxi_open_attenuator(CXI_Attenuator_Reference * ref){
  cxi_debug("opening attenuator");
  if(!ref){
    return NULL;
  }
  CXI_Attenuator * attenuator = calloc(sizeof(CXI_Attenuator),1);
  if(!attenuator){
    return NULL;
  }

  attenuator->handle = H5Gopen(ref->parent_handle,ref->group_name,H5P_DEFAULT);
  if(attenuator->handle < 0){
    free(attenuator);
    return NULL;
  }
  ref->attenuator = attenuator;  
  attenuator->distance_valid = try_read_float(attenuator->handle, "distance",&attenuator->distance);
  attenuator->thickness_valid = try_read_float(attenuator->handle, "thickness",&attenuator->thickness);
  attenuator->attenuator_transmission_valid = try_read_float(attenuator->handle, "attenuator_transmission",
							     &attenuator->attenuator_transmission);
  try_read_string(attenuator->handle, "type",&attenuator->type);


  return attenuator;
}

static void cxi_close_attenuator(CXI_Attenuator_Reference * ref){
  cxi_debug("closing attenuator");
  CXI_Attenuator * attenuator = ref->attenuator;
  if(attenuator){
    H5Gclose(attenuator->handle);
    free(attenuator->type);
    free(attenuator);
  }
  free(ref->group_name);
  free(ref);
}


CXI_Monochromator * cxi_open_monochromator(CXI_Monochromator_Reference * ref){
  cxi_debug("opening monochromator");
  if(!ref){
    return NULL;
  }
  CXI_Monochromator * monochromator = calloc(sizeof(CXI_Monochromator),1);
  if(!monochromator){
    return NULL;
  }

  monochromator->handle = H5Gopen(ref->parent_handle,ref->group_name,H5P_DEFAULT);
  if(monochromator->handle < 0){
    free(monochromator);
    return NULL;
  }
  ref->monochromator = monochromator;  
  monochromator->energy_valid = try_read_float(monochromator->handle, "energy",&monochromator->energy);
  monochromator->energy_error = try_read_float(monochromator->handle, "energy_error",&monochromator->energy_error);
  return monochromator;
}

static void cxi_close_monochromator(CXI_Monochromator_Reference * ref){
  cxi_debug("closing monochromator");
  CXI_Monochromator * monochromator = ref->monochromator;
  if(monochromator){
    H5Gclose(monochromator->handle);
    free(monochromator);
  }
  free(ref->group_name);
  free(ref);
}


CXI_Image * cxi_open_image(CXI_Image_Reference * ref){
  cxi_debug("opening image");
  char buffer[1024];
  if(!ref){
    return NULL;
  }
  CXI_Image * image = calloc(sizeof(CXI_Image),1);
  if(!image){
    return NULL;
  }

  image->handle = H5Gopen(ref->parent_handle,ref->group_name,H5P_DEFAULT);
  if(image->handle < 0){
    free(image);
    return NULL;
  }

  /* Search for Detector groups */
  int n = find_max_suffix(image->handle, "detector");
  image->detector_count = n;
  image->detectors = calloc(sizeof(CXI_Detector_Reference *),n);
  for(int i = 0;i<n;i++){
    image->detectors[i] = calloc(sizeof(CXI_Detector_Reference),1);
    sprintf(buffer,"detector_%d",i+1);
    image->detectors[i]->parent_handle = image->handle;
    image->detectors[i]->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(image->detectors[i]->group_name,buffer);      
  }

  ref->image = image;  
  if(H5Lexists(image->handle,"data",H5P_DEFAULT)){
    image->data = calloc(sizeof(CXI_Dataset_Reference),1);
    image->data->parent_handle = image->handle;
    image->data->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(image->data->group_name,"data");          
  }

  if(H5Lexists(image->handle,"data_error",H5P_DEFAULT)){
    image->data_error = calloc(sizeof(CXI_Dataset_Reference),1);
    image->data_error->parent_handle = image->handle;
    image->data_error->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(image->data_error->group_name,"data_error");          
  }


  if(H5Lexists(image->handle,"mask",H5P_DEFAULT)){
    image->mask = calloc(sizeof(CXI_Dataset_Reference),1);
    image->mask->parent_handle = image->handle;
    image->mask->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(image->mask->group_name,"mask");      
    
  }

  //  try_read_string(image->handle, "data_space",&image->data_space);
  //  try_read_string(image->handle, "data_type",&image->data_type);
  image->dimensionality_valid = try_read_int(image->handle, "dimensionality",&image->dimensionality);
  image->image_center_valid = try_read_float_array(image->handle, "image_center",image->image_center,3);

  return image;
}

static void cxi_close_image(CXI_Image_Reference * ref){
  cxi_debug("closing image");
  CXI_Image * image = ref->image;
  if(image){
    H5Gclose(image->handle);
    free(image);
  }
  free(ref->group_name);
  free(ref);
}


CXI_Sample * cxi_open_sample(CXI_Sample_Reference * ref){
  cxi_debug("opening sample");
  if(!ref){
    return NULL;
  }
  CXI_Sample * sample = calloc(sizeof(CXI_Sample),1);
  if(!sample){
    return NULL;
  }

  sample->handle = H5Gopen(ref->parent_handle,ref->group_name,H5P_DEFAULT);
  if(sample->handle < 0){
    free(sample);
    return NULL;
  }
  ref->sample = sample;  
  return sample;
}

static void cxi_close_sample(CXI_Sample_Reference * ref){
  cxi_debug("closing sample");
  CXI_Sample * sample = ref->sample;
  if(sample){
    H5Gclose(sample->handle);
    free(sample);
  }
  free(ref->group_name);
  free(ref);
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

int cxi_read_dataset_slice(CXI_Dataset * dataset, unsigned int slice, void * data, hid_t datatype){
  if(!dataset){
    return -1;
  }
  if(!data){
    return -1;
  }
  if(slice >= dataset->dimensions[0]){
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


CXI_Entry_Reference * cxi_write_entry(hid_t loc, CXI_Entry * entry){
  if(loc < 0 || !entry){
    return NULL;
  }
  /* Check date format */
  if(entry->end_time && !follows_iso8601(entry->end_time)){
    cxi_debug("CXI_Entry->end_time: %s does not follow ISO8601",entry->end_time);
    return NULL;
  }
  if(entry->start_time && !follows_iso8601(entry->start_time)){
    cxi_debug("CXI_Entry->start_time: %s does not follow ISO8601",entry->start_time);
    return NULL;
  }

  cxi_debug("writing entry");  
  int n = find_max_suffix(loc, "entry");
  char buffer[1024];
  sprintf(buffer,"entry_%d",n+1);
  hid_t handle = H5Gcreate(loc,buffer, H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  if(handle < 0){
    return NULL;
  }
  CXI_Entry_Reference * ref = calloc(sizeof(CXI_Entry_Reference),1);
  entry->handle = handle;
  ref->parent_handle = loc;
  ref->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
  ref->entry = entry;
  strcpy(ref->group_name,buffer);
  /* Write down all the scalars that are in the entry */
  if(entry->end_time){
    try_write_string(entry->handle, "end_time", entry->end_time);
  }
  if(entry->experiment_identifier){
    try_write_string(entry->handle, "experiment_identifier", entry->experiment_identifier);
  }
  if(entry->experiment_description){
    try_write_string(entry->handle, "experiment_description", entry->experiment_description);
  }
  if(entry->program_name){
    try_write_string(entry->handle, "program_name", entry->program_name);
  }

  if(entry->start_time){
    try_write_string(entry->handle, "start_time", entry->start_time);
  }
  if(entry->title){
    try_write_string(entry->handle, "title", entry->title);
  }
  if(entry->data){
    for(int i = 0;i<entry->data_count;i++){
      if(entry->data[i]->data){
	cxi_write_data(entry->handle,entry->data[i]->data);
      }else{
	cxi_debug("Warning: Data Reference data[%d] has NULL data pointer\n", i);
      }
    }
  }
  if(entry->images){
    for(int i = 0;i<entry->image_count;i++){
      if(entry->images[i]->image){
	cxi_write_image(entry->handle,entry->images[i]->image);
      }else{
	cxi_debug("Warning: Image Reference images[%d] has NULL image pointer\n", i);
      }
    }
  }
  if(entry->instruments){
    for(int i = 0;i<entry->instrument_count;i++){
      if(entry->instruments[i]->instrument){
	cxi_write_instrument(entry->handle,entry->instruments[i]->instrument);
      }else{
	cxi_debug("Warning: Instrument Reference instruments[%d] has NULL instrument pointer\n", i);
      }
    }
  }

  if(entry->samples){
    for(int i = 0;i<entry->sample_count;i++){
      if(entry->samples[i]->sample){
	cxi_write_sample(entry->handle,entry->samples[i]->sample);
      }else{
	cxi_debug("Warning: Sample Reference samples[%d] has NULL sample pointer\n", i);
      }
    }
  }
  return ref;  
}

CXI_Data_Reference * cxi_write_data(hid_t loc, CXI_Data * data){
  if(loc < 0 || !data){
    return NULL;
  }
  int n = find_max_suffix(loc, "data");
  char buffer[1024];
  sprintf(buffer,"data_%d",n+1);
  hid_t handle = H5Gcreate(loc,buffer, H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  if(handle < 0){
    return NULL;
  }
  CXI_Data_Reference * ref = calloc(sizeof(CXI_Data_Reference),1);
  data->handle = handle;
  ref->parent_handle = loc;
  ref->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
  ref->data = data;
  strcpy(ref->group_name,buffer);
  return ref;

}
CXI_Image_Reference * cxi_write_image(hid_t loc, CXI_Image * image){
  if(loc < 0 || !image){
    return NULL;
  }
  int n = find_max_suffix(loc, "image");
  char buffer[1024];
  sprintf(buffer,"image_%d",n+1);
  hid_t handle = H5Gcreate(loc,buffer, H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  if(handle < 0){
    return NULL;
  }
  CXI_Image_Reference * ref = calloc(sizeof(CXI_Image_Reference),1);
  image->handle = handle;
  ref->parent_handle = loc;
  ref->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
  ref->image = image;
  strcpy(ref->group_name,buffer);
  return ref;

}
CXI_Instrument_Reference * cxi_write_instrument(hid_t loc, CXI_Instrument * instrument){
  if(loc < 0 || !instrument){
    return NULL;
  }
  int n = find_max_suffix(loc, "instrument");
  char buffer[1024];
  sprintf(buffer,"instrument_%d",n+1);
  hid_t handle = H5Gcreate(loc,buffer, H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  if(handle < 0){
    return NULL;
  }
  CXI_Instrument_Reference * ref = calloc(sizeof(CXI_Instrument_Reference),1);
  instrument->handle = handle;
  ref->parent_handle = loc;
  ref->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
  ref->instrument = instrument;
  strcpy(ref->group_name,buffer);
  return ref;

}
CXI_Sample_Reference * cxi_write_sample(hid_t loc, CXI_Sample * sample){
  if(loc < 0 || !sample){
    return NULL;
  }
  int n = find_max_suffix(loc, "sample");
  char buffer[1024];
  sprintf(buffer,"sample_%d",n+1);
  hid_t handle = H5Gcreate(loc,buffer, H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  if(handle < 0){
    return NULL;
  }
  CXI_Sample_Reference * ref = calloc(sizeof(CXI_Sample_Reference),1);
  sample->handle = handle;
  ref->parent_handle = loc;
  ref->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
  ref->sample = sample;
  strcpy(ref->group_name,buffer);
  if(sample->concentration_valid){
    try_write_float(handle,"concentration",sample->concentration);
  }
  if(sample->description){
    try_write_string(handle,"description",sample->description);
  }
  if(sample->mass_valid){
    try_write_float(handle,"mass",sample->mass);
  }
  if(sample->name){
    try_write_string(handle,"name",sample->name);
  }

  if(sample->temperature){
    try_write_float(handle,"temperature",sample->temperature);
  }

  if(sample->unit_cell_valid){
    try_write_float_2D_array(handle,"unit_cell",(double *)sample->unit_cell,2,3);
  }
  if(sample->unit_cell_group){
    try_write_string(handle,"unit_cell_group",sample->unit_cell_group);
  }
  if(sample->thickness_valid){
    try_write_float(handle,"thickness",sample->thickness);
  }
  if(sample->unit_cell_volume_valid){
    try_write_float(handle,"unit_cell_volume",sample->unit_cell_volume);
  }


  return ref;

}

CXI_Monochromator_Reference * cxi_write_monochromator(hid_t loc, CXI_Monochromator * monochromator){
  if(loc < 0 || !monochromator){
    return NULL;
  }
  int n = find_max_suffix(loc, "monochromator");
  char buffer[1024];
  sprintf(buffer,"monochromator_%d",n+1);
  hid_t handle = H5Gcreate(loc,buffer, H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  if(handle < 0){
    return NULL;
  }
  CXI_Monochromator_Reference * ref = calloc(sizeof(CXI_Monochromator_Reference),1);
  monochromator->handle = handle;
  ref->parent_handle = loc;
  ref->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
  ref->monochromator = monochromator;
  strcpy(ref->group_name,buffer);
  return ref;

}


CXI_Attenuator_Reference * cxi_write_attenuator(hid_t loc, CXI_Attenuator * attenuator){
  if(loc < 0 || !attenuator){
    return NULL;
  }
  int n = find_max_suffix(loc, "attenuator");
  char buffer[1024];
  sprintf(buffer,"attenuator_%d",n+1);
  hid_t handle = H5Gcreate(loc,buffer, H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  if(handle < 0){
    return NULL;
  }
  CXI_Attenuator_Reference * ref = calloc(sizeof(CXI_Attenuator_Reference),1);
  attenuator->handle = handle;
  ref->parent_handle = loc;
  ref->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
  ref->attenuator = attenuator;
  strcpy(ref->group_name,buffer);
  return ref;
}

CXI_Source_Reference * cxi_write_source(hid_t loc, CXI_Source * source){
  if(loc < 0 || !source){
    return NULL;
  }
  int n = find_max_suffix(loc, "source");
  char buffer[1024];
  sprintf(buffer,"source_%d",n+1);
  hid_t handle = H5Gcreate(loc,buffer, H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  if(handle < 0){
    return NULL;
  }
  CXI_Source_Reference * ref = calloc(sizeof(CXI_Source_Reference),1);
  source->handle = handle;
  ref->parent_handle = loc;
  ref->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
  ref->source = source;
  strcpy(ref->group_name,buffer);
  if(source->energy_valid){
    try_write_float(handle,"energy",source->energy);
  }
  if(source->pulse_energy_valid){
    try_write_float(handle,"pulse_energy",source->pulse_energy);
  }
  if(source->pulse_width_valid){
    try_write_float(handle,"pulse_width",source->pulse_width);
  }
  return ref;

}

CXI_Detector_Reference * cxi_write_detector(hid_t loc, CXI_Detector * detector){
  if(loc < 0 || !detector){
    return NULL;
  }
  int n = find_max_suffix(loc, "detector");
  char buffer[1024];
  sprintf(buffer,"detector_%d",n+1);
  hid_t handle = H5Gcreate(loc,buffer, H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  if(handle < 0){
    return NULL;
  }
  CXI_Detector_Reference * ref = calloc(sizeof(CXI_Detector_Reference),1);
  detector->handle = handle;
  ref->parent_handle = loc;
  ref->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
  ref->detector = detector;
  strcpy(ref->group_name,buffer);
  if(detector->basis_vectors_valid){
    try_write_float_2D_array(handle,"basis_vectors",(double *)detector->basis_vectors,2,3);
  }
  if(detector->corner_position_valid){
    try_write_float_1D_array(handle,"corner_position",(double *)detector->corner_position,3);
  }
  if(detector->counts_per_joule_valid){
    try_write_float(handle,"counts_per_joule",detector->counts_per_joule);
  }
  if(detector->data_sum_valid){
    try_write_float(handle,"data_sum",detector->data_sum);
  }
  if(detector->description){
    try_write_string(handle,"description",detector->description);
  }
  if(detector->distance_valid){
    try_write_float(handle,"distance",detector->distance);
  }
  if(detector->x_pixel_size_valid){
    try_write_float(handle,"x_pixel_size",detector->x_pixel_size);
  }
  if(detector->y_pixel_size_valid){
    try_write_float(handle,"y_pixel_size",detector->y_pixel_size);
  }
  return ref;
}

CXI_Geometry_Reference * cxi_write_geometry(hid_t loc, CXI_Geometry * geometry){
  if(loc < 0 || !geometry){
    return NULL;
  }
  int n = find_max_suffix(loc, "geometry");
  char buffer[1024];
  sprintf(buffer,"geometry_%d",n+1);
  hid_t handle = H5Gcreate(loc,buffer, H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  if(handle < 0){
    return NULL;
  }
  CXI_Geometry_Reference * ref = calloc(sizeof(CXI_Geometry_Reference),1);
  geometry->handle = handle;
  ref->parent_handle = loc;
  ref->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
  ref->geometry = geometry;
  strcpy(ref->group_name,buffer);
  return ref;
}

CXI_Process_Reference * cxi_write_process(hid_t loc, CXI_Process * process){
  if(loc < 0 || !process){
    return NULL;
  }
  int n = find_max_suffix(loc, "process");
  char buffer[1024];
  sprintf(buffer,"process_%d",n+1);
  hid_t handle = H5Gcreate(loc,buffer, H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  if(handle < 0){
    return NULL;
  }
  CXI_Process_Reference * ref = calloc(sizeof(CXI_Process_Reference),1);
  process->handle = handle;
  ref->parent_handle = loc;
  ref->group_name = malloc(sizeof(char)*(strlen(buffer)+1));
  ref->process = process;
  strcpy(ref->group_name,buffer);
  return ref;
}

static char * dataset_type_to_name(CXI_Dataset_Type type){  
  static char * table[] = {
    "data",
    "data_dark",
    "data_white",
    "data_error",
    "errors",
    "mask",
    "reciprocal_coordinates"    
  };  
  return table[type];
  
}

CXI_Dataset_Reference * cxi_create_dataset(hid_t loc, CXI_Dataset * dataset, 
					   CXI_Dataset_Type type){
  if(loc < 0 || !dataset){
    return NULL;
  }
  char * name = dataset_type_to_name(type);
  hid_t dataspace = H5Screate_simple(dataset->dimension_count,
				     dataset->dimensions, NULL );
  hid_t handle = H5Dcreate(loc,name, dataset->data_type, dataspace,  
			   H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  if(handle < 0){
    return NULL;
  }
  CXI_Dataset_Reference * ref = calloc(sizeof(CXI_Dataset_Reference),1);
  dataset->handle = handle;
  ref->parent_handle = loc;
  ref->group_name = malloc(sizeof(char)*(strlen(name)+1));
  ref->dataset = dataset;
  strcpy(ref->group_name,name);
  return ref;
}

int cxi_write_dataset(CXI_Dataset * dataset, void * data, hid_t datatype){
  if(!dataset){
    return -1;
  }
  if(!data){
    return -1;
  }
  if(dataset->handle < 0){
    return -1;
  }
  H5Dwrite(dataset->handle,datatype,H5S_ALL,H5S_ALL,H5P_DEFAULT,data);      
  return 0;
}

int cxi_write_dataset_slice(CXI_Dataset * dataset,unsigned int slice, void * data, hid_t datatype){
  if(!dataset){
    return -1;
  }
  if(!data){
    return -1;
  }
  if(slice >= dataset->dimensions[0]){
    return -1;
  }
  if(dataset->handle < 0){
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
  H5Dwrite(dataset->handle,datatype,memspace,s,H5P_DEFAULT,data);      
  H5Sclose(s);
  free(start);
  free(count);

  return 0;
}

hsize_t cxi_dataset_length(CXI_Dataset * dataset){
  if(!dataset){
    return 0;
  }
  if(!dataset->dimensions){
    return 0;
  }
  if(dataset->dimension_count <= 0){
    return 0;
  }
  hsize_t ret = 1;
  for(int i = 0;i<dataset->dimension_count;i++){
    ret *= dataset->dimensions[i];
  }
  return ret;
}
