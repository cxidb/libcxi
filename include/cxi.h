#pragma once 

#include <hdf5.h>

#ifdef __cplusplus 
extern "C"{
#endif

  typedef enum{
    CXI_Data_Type,
    CXI_Data_Dark_Type,
    CXI_Data_White_Type,
    CXI_Data_Error_Type,
    CXI_Errors_Type,
    CXI_Mask_Type,
    CXI_Reciprocal_Coordinates_Type    
  }CXI_Dataset_Type;


  typedef struct _CXI_Dataset{
    hid_t handle;
    hsize_t * dimensions;
    int dimension_count;
    hsize_t size;
    hsize_t slice_size;
    hsize_t slice_count;
    hid_t datatype;    
  }CXI_Dataset;

  typedef struct _CXI_Dataset_Reference{
    hid_t parent_handle;
    char * group_name;
    CXI_Dataset * dataset;
  }CXI_Dataset_Reference;


  typedef struct _CXI_Process{
    hid_t handle;
  }CXI_Process;

  typedef struct _CXI_Process_Reference{
    hid_t parent_handle;
    char * group_name;
    CXI_Process * process;
  }CXI_Process_Reference;
  
  typedef struct _CXI_Attenuator{
    hid_t handle;

    double distance;
    int distance_valid;

    double thickness;
    int thickness_valid;

    double attenuator_transmission;
    int attenuator_transmission_valid;

    char * type;
  }CXI_Attenuator;

typedef struct _CXI_Attenuator_Reference{
  hid_t parent_handle;
  char * group_name;
  CXI_Attenuator * attenuator;
}CXI_Attenuator_Reference;

  typedef struct _CXI_Geometry{
    hid_t handle;

    double orientation[2][3];
    int orientation_valid;

    double translation[3];    
    int translation_valid;
  }CXI_Geometry;

  typedef struct _CXI_Geometry_Reference{
    hid_t parent_handle;
    char * group_name;
    CXI_Geometry * geometry;
  }CXI_Geometry_Reference;

  typedef struct _CXI_Detector{
    hid_t handle;

    double basis_vectors[2][3];
    int basis_vectors_valid;

    double corner_position[3];
    int corner_position_valid;

    double counts_per_joule;
    int counts_per_joule_valid;

    CXI_Dataset_Reference * data;
    CXI_Dataset_Reference * data_dark;
    CXI_Dataset_Reference * data_white;
    CXI_Dataset_Reference * data_error;

    double data_sum;
    int data_sum_valid;

    char * description;

    double distance;
    int distance_valid;

    CXI_Geometry_Reference * geometry;

    CXI_Dataset_Reference * mask;

    double x_pixel_size;
    int x_pixel_size_valid;

    double y_pixel_size;
    int y_pixel_size_valid;
  }CXI_Detector;

  typedef struct _CXI_Detector_Reference{
    hid_t parent_handle;
    char * group_name;
    CXI_Detector * detector;
  }CXI_Detector_Reference;


  typedef struct _CXI_Source{
    hid_t handle;

    /* in Joules per photon! */
    double energy;
    int energy_valid;

    char * name;
    /* in Joules per pulse! */
    double pulse_energy;
    int pulse_energy_valid;

    /* in seconds */
    double pulse_width;
    int pulse_width_valid;
  }CXI_Source;

  typedef struct _CXI_Source_Reference{
    hid_t parent_handle;
    char * group_name;
    CXI_Source * source;
  }CXI_Source_Reference;


  typedef struct _CXI_Monochromator{
    hid_t handle;

    double energy;
    int energy_valid;

    double energy_error;
    int energy_error_valid;
  }CXI_Monochromator;
  
typedef struct _CXI_Monochromator_Reference{
  hid_t parent_handle;
  char * group_name;
  CXI_Monochromator * monochromator;
}CXI_Monochromator_Reference;

  typedef struct _CXI_Instrument{
    hid_t handle;

    char * name;
    CXI_Attenuator_Reference ** attenuators;
    int attenuator_count;
    CXI_Detector_Reference ** detectors;
    int detector_count;
    CXI_Monochromator_Reference ** monochromators;
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

    CXI_Dataset_Reference * data;
    CXI_Dataset_Reference * errors;    
  }CXI_Data;

  typedef struct _CXI_Data_Reference{
    hid_t parent_handle;
    char * group_name;
    CXI_Data * data;
  }CXI_Data_Reference;

  typedef struct _CXI_Image{
    hid_t handle;

    CXI_Dataset_Reference * data;
    CXI_Dataset_Reference * data_error;
    char * data_space;
    char * data_type;

    CXI_Detector_Reference ** detectors;
    int detector_count;

    int dimensionality;
    int dimensionality_valid;

    double image_center[3];
    int image_center_valid;

    double image_size[3];
    int image_size_valid;

    int is_fft_shifted;
    int is_fft_shifted_valid;

    CXI_Dataset_Reference * mask;

    CXI_Process_Reference ** processs;
    int process_count;

    CXI_Dataset_Reference * reciprocal_coordinates;

    CXI_Source_Reference ** sources;
    int source_count;    
  }CXI_Image;
  
  typedef struct _CXI_Image_Reference{
    hid_t parent_handle;
    char * group_name;
    CXI_Image * image;
  }CXI_Image_Reference;
  
  typedef struct _CXI_Sample{
    hid_t handle;

    float concentration;
    int concentration_valid;

    char * description;
    
    CXI_Geometry_Reference * geometry;

    float mass;
    int mass_valid;

    char * name;
    
    float temperature;
    int temperature_valid;
    
    float unit_cell[2][3];
    int unit_cell_valid;
    
    char * unit_cell_group;

    float thickness;
    int thickness_valid;
    
    float unit_cell_volume;
    int unit_cell_volume_valid;
  }CXI_Sample;

  typedef struct _CXI_Sample_Reference{
    hid_t parent_handle;
    char * group_name;
    CXI_Sample * sample;
  }CXI_Sample_Reference;

  typedef struct _CXI_Entry{
    hid_t handle;

    /* If any of members is not read then it is set to 0 for pointers */
    CXI_Data_Reference ** data;
    int data_count;

    char * end_time;
    char * experiment_identifier;
    char * experiment_description;

    CXI_Image_Reference ** images;
    int image_count;

    CXI_Instrument_Reference ** instruments;
    int instrument_count;

    char * program_name;

    CXI_Sample_Reference ** samples;
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
  int cxi_close_file(CXI_File * file);

  CXI_Entry * cxi_open_entry(CXI_Entry_Reference * entry);
  CXI_Instrument * cxi_open_instrument(CXI_Instrument_Reference * instrument);
  CXI_Detector * cxi_open_detector(CXI_Detector_Reference * detector);
  CXI_Attenuator * cxi_open_attenuator(CXI_Attenuator_Reference * attenuator);
  CXI_Monochromator * cxi_open_monochromator(CXI_Monochromator_Reference * monochromator);
  CXI_Sample * cxi_open_sample(CXI_Sample_Reference * sample);
  CXI_Source * cxi_open_source(CXI_Source_Reference * source);
  CXI_Data * cxi_open_data(CXI_Data_Reference * ref);
  CXI_Dataset * cxi_open_dataset(CXI_Dataset_Reference * ref);
  

  int cxi_read_dataset(CXI_Dataset * dataset, void * data, hid_t datatype);
  int cxi_read_dataset_slice(CXI_Dataset * dataset, unsigned int slice, void * data, hid_t datatype);


  CXI_Entry_Reference * cxi_write_entry(hid_t loc, CXI_Entry * entry);
  CXI_Data_Reference * cxi_write_data(hid_t loc, CXI_Data * data);
  CXI_Image_Reference * cxi_write_image(hid_t loc, CXI_Image * image);
  CXI_Instrument_Reference * cxi_write_instrument(hid_t loc, CXI_Instrument * instrument);
  CXI_Sample_Reference * cxi_write_sample(hid_t loc, CXI_Sample * sample);
  CXI_Attenuator_Reference * cxi_write_attenuator(hid_t loc, CXI_Attenuator * attenuator);
  CXI_Monochromator_Reference * cxi_write_monochromator(hid_t loc, CXI_Monochromator * monochromator);
  CXI_Source_Reference * cxi_write_source(hid_t loc, CXI_Source * source);
  CXI_Process_Reference * cxi_write_process(hid_t loc, CXI_Process * process);

  CXI_Dataset_Reference * cxi_create_dataset(hid_t loc, CXI_Dataset * dataset, 
					     CXI_Dataset_Type type);

  int cxi_write_dataset(CXI_Dataset * dataset, void * data, hid_t datatype);
  int cxi_write_dataset_slice(CXI_Dataset * dataset, unsigned int slice, void * data, hid_t datatype);

#ifdef __cplusplus 
} /* extern "C" */
#endif  

