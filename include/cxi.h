#pragma once 

#include <hdf5.h>

#ifdef __cplusplus 
extern "C"{
#endif


  typedef struct _CXI_Dataset{
    hid_t handle;
    hid_t parent_handle;
    hsize_t * dimensions;
    int dimension_count;
    hsize_t size;
    hsize_t slice_size;
    hid_t datatype;    
  }CXI_Dataset;


  typedef struct _CXI_Process{
    hid_t handle;
    hid_t parent_handle;
    char * group_name;
  }CXI_Process;

  typedef struct _CXI_Attenuator{
    hid_t handle;
    hid_t parent_handle;
    char * group_name;
  }CXI_Attenuator;

  typedef struct _CXI_Geometry{
    hid_t handle;
    hid_t parent_handle;
    char * group_name;

    double orientation[2][3];
    double translation[3];    
  }CXI_Geometry;

  typedef struct _CXI_Geometry_Reference{
    hid_t parent_handle;
    char * group_name;
    CXI_Geometry * detector;
  }CXI_Geometry_Reference;

  typedef struct _CXI_Detector{
    hid_t handle;
    hid_t parent_handle;
    char * group_name;

    double basis_vectors[3][3];
    double corner_position[3];
    double counts_per_joule;
    CXI_Dataset * data;
    CXI_Dataset * data_dark;
    CXI_Dataset * data_white;
    CXI_Dataset * data_error;
    double data_sum;
    char * description;
    double distance;
    CXI_Geometry ** geometries;
    int geometry_count;
    CXI_Dataset * mask;
    double x_pixel_size;
    double y_pixel_size;
  }CXI_Detector;

  typedef struct _CXI_Detector_Reference{
    hid_t parent_handle;
    char * group_name;
    CXI_Detector * detector;
  }CXI_Detector_Reference;


  typedef struct _CXI_Source{
    hid_t handle;
    hid_t parent_handle;
    char * group_name;

    /* in Joules per photon! */
    double energy;
    char * name;
    /* in Joules per pulse! */
    double pulse_energy;
    /* in seconds */
    double pulse_width;
  }CXI_Source;

  typedef struct _CXI_Source_Reference{
    hid_t parent_handle;
    char * group_name;
    CXI_Source * source;
  }CXI_Source_Reference;


  typedef struct _CXI_Monochromator{
    hid_t handle;
    hid_t parent_handle;
    char * group_name;

  }CXI_Monochromator;
  

  typedef struct _CXI_Instrument{
    hid_t handle;
    hid_t parent_handle;
    char * group_name;

    char * name;
    CXI_Attenuator ** attenuators;
    int attenuator_count;
    CXI_Detector_Reference ** detectors;
    int detector_count;
    CXI_Monochromator ** monochromators;
    int monochromator_count;
    CXI_Source_Reference ** sources;
    int source_count;
  }CXI_Instrument;

  typedef struct _CXI_Instrument_Reference{
    hid_t parent_handle;
    char * group_name;
    CXI_Instrument * instrument;
  }CXI_Instrument_Reference;

  typedef struct _CXI_Data{
    hid_t handle;
    hid_t parent_handle;
    char * group_name;

    CXI_Dataset * data;
  }CXI_Data;

  typedef struct _CXI_Data_Reference{
    hid_t parent_handle;
    char * group_name;
    CXI_Data * data;
  }CXI_Data_Reference;

  typedef struct _CXI_Image{
    hid_t handle;
    hid_t parent_handle;
    char * group_name;

    CXI_Dataset * data;
    CXI_Dataset * data_error;
    char * data_space;
    char * data_type;
    CXI_Detector ** detectors;
    int detector_count;
    int dimensionality;
    double image_center[3];
    double image_size[3];
    int is_fft_shifted;
    CXI_Dataset * mask;
    CXI_Process ** processs;
    int process_count;
    CXI_Dataset * reciprocal_coordinates;
    CXI_Source ** sources;
    int source_count;    
  }CXI_Image;


  typedef struct _CXI_Sample{
    hid_t handle;
    hid_t parent_handle;
    char * group_name;
  }CXI_Sample;


  typedef struct _CXI_Entry{
    hid_t handle;
    hid_t parent_handle;
    char * group_name;
    /* If any of members is not read then it is set to 0 for pointers */
    CXI_Data ** data;
    int data_count;
    char * end_time;
    char * experiment_identifier;
    char * experiment_description;
    CXI_Image ** images;
    int image_count;
    CXI_Instrument_Reference ** instruments;
    int instrument_count;
    char * program_name;
    CXI_Sample ** samples;
    int sample_count;
    char * start_time;
    char * title;
  }CXI_Entry;

  typedef struct _CXI_Entry_Reference{
    hid_t parent_handle;
    char * group_name;
    CXI_Entry * entry;
  }CXI_Entry_Reference;

  typedef struct _CXI_File{
    hid_t handle;
    char * filename;
    CXI_Entry_Reference ** entries;
    int entry_count;
  }CXI_File;


  /* Open file for reading or writing a CXI */
  CXI_File * cxi_open_file(const char * filename, const char * mode);
  int cxi_close(CXI_File * file);

  CXI_Entry * cxi_open_entry(CXI_Entry_Reference * entry);
  CXI_Instrument * cxi_open_instrument(CXI_Instrument_Reference * instrument);
  CXI_Detector * cxi_open_detector(CXI_Detector_Reference * detector);
  CXI_Source * cxi_open_source(CXI_Source_Reference * source);

  int cxi_read_dataset(CXI_Dataset * dataset, void * data, hid_t datatype);
  int cxi_read_dataset_slice(CXI_Dataset * dataset, int slice, void * data, hid_t datatype);

#ifdef __cplusplus 
} /* extern "C" */
#endif

/*
  Example:

  CXI_File * file = cxi_open("foo.cxi","r");
  CXI_Entry * entry = cxi_open_entry(file,1);
  CXI_Instrument * instrument = cxi_open_instrument(entry, 1);
  CXI_Detector * detector = cxi_open_detector(instrument, 1);
  short * image = malloc(sizeof(short)*detector->data->total_dimension);
  cxi_read_dataset(detector->data, image, H5T_NATIVE_SHORT);
  cxi_close(file);
*/
  

