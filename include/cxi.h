/*! \mainpage 
 *
 * \section intro_sec Introduction
 *
 * \p <span class="orange">lib</span><span class="blue">cxi</span> is a C library intended to help you to read and write [CXI files](http://cxidb.org/cxi.html).
 * 
 *
 * \section install_sec 1. Installation
 *
 * \subsection step1 Step 1: Obtaining the source
 *
 * You can obtain the source code of <span class="orange">lib</span><span class="blue">cxi</span> from [GitHub](https://github.com/FilipeMaia/libcxi).
 * 
 * Using \p git you can do:
~~~{.sh}
 git clone https://github.com/FilipeMaia/libcxi.git
~~~
 *
 * Alternatively you can directly [download the latest verion](https://github.com/FilipeMaia/libcxi/archive/master.zip), in zip format.
 * 
 * \subsection step2 Step 2: Building the library
 *
 * - Make sure you have HDF5 installed in your machine.
 * - Create a \p build directory inside the source code root directory.
 * - Inside the build directory run:
 ~~~{.sh}
 ccmake .. 
 ~~~
 * - If all the necessary dependencies are found (HDF5) you can just press `c` to configure followed by `g` to generate the Makefiles.
 * - Now just run `make` to actually build the library. If you get any error please post then on the <span class="orange">lib</span><span class="blue">cxi</span> [issues page](https://github.com/FilipeMaia/libcxi/issues)
 * - The library should now be built inside the \p src directory.
 *
 *
 * \section reading_cxi 2. Reading CXI files
 *
 * \subsection reading_minimal_cxi 2.1 Reading a minimal CXI file
 *
 * <span class="orange">lib</span><span class="blue">cxi</span> implements a hierarchichal approach to
 * reading CXI files, opening only one group of the HDF5 tree at a time. First we'll try to read a minimal
 * CXI file, which is structured as shown in the image below. 
 * ![Fig 1. Diagram of a minimal CXI file with a single image](@ref minimal_cxi.png)
 *
 *
 * The following example shows how to read such a file:
 *
 * \include examples/minimal_reader.c
 *
 * \subsection reading_typical_cxi 2.2 Reading a more typical CXI file
 * Here we show how to read a more typical CXI file.
 * ![Fig 2. Diagram of a typical CXI file for storing raw data from a single shot.](@ref lcls_camp_cxi.png)
 *
 * And now the code to read it:
 * \include examples/typical_reader.c
 *
 * \section writing_cxi 3. Writing CXI files
 *
 * \subsection writing_minimal_cxi 3.1 Writing a minimal CXI file
 *
 * \include examples/minimal_writer.c
 *
 * \subsection writing_typical_cxi 3.2 Writing a typical CXI file
 *
 * \include examples/typical_writer.c
 *
 */
#pragma once 

#include <hdf5.h>

#ifdef __cplusplus 
extern "C"{
#endif

/*! \file cxi.h
 * 
 *  Contains all the data structures and functions.
 */

/*! \p CXI_Dataset_Type enumerates the possible types of datasets.
 *  It's is used for example when creating a new dataset. CXI_Data_Type is a general
 *  type while the other ones are used for specific datasets, which are obvious from
 *  the name.
 *
 * \see cxi_create_dataset
 */
  typedef enum {
    /*! General data type */
    CXI_Data_Type, 
    /*! Image recorded with the shutter closed, used for background subtractions */
    CXI_Data_Dark_Type,
    /*! Image recorded without the sample, used for background subtractions. */
    CXI_Data_White_Type,
    /*! The best estimate of the uncertainty in the data value. Where possible, this should be the standard deviation, which has the same units as the data.*/ 
    CXI_Data_Error_Type,
    /* MISSING DOC */
    CXI_Errors_Type, 
    /*! A mask for an image. \see CXI_Image::mask */
    CXI_Mask_Type, 
    /*! The diffraction (Fourier) space coordinates of the center of each pixel. \see CXI_Image::reciprocal_coordinates */
    CXI_Reciprocal_Coordinates_Type     
  }CXI_Dataset_Type;

  /*! Possible spaces for an image. 
   */
  typedef enum{
    /*! To be used when the value is not set */
    CXI_Invalid_Space = 0, 
    /*! Real space image */
    CXI_Real_Space, 
    /*! Diffraction (Fourier) space image */
    CXI_Diffraction_Space  
  }CXI_Image_Space;

  /*! Possible types of image data. */
  typedef enum{
    /*! To be used when the value is not set */
    CXI_Invalid_Type = 0, 
    /*! Intensities */
    CXI_Intensity_Type, 
    /*! Electron densities */
    CXI_Electron_Density_Type, 
    /*! Phased amplitudes */
    CXI_Amplitude_Type, 
    /*! Unphased amplitudes, the square root of intensities */
    CXI_Unphased_Amplitude_Type, 
    /*! Autocorrelation */
    CXI_Autocorrelation_Type 
  }CXI_Image_Type;

  /*! Number of dimensions of the image data.
    When unset use \p CXI_Invalid_Dimensional. */
  typedef enum{
    /*! To be used when the value is not set */
    CXI_Invalid_Dimensional = 0, 
    /*! One dimensional images */
    CXI_One_Dimensional, 
    /*! Two dimensional images */
    CXI_Two_Dimensional, 
    /*! Three dimensional images */
    CXI_Three_Dimensional 
  }CXI_Image_Dimensionality;


  /*! Defines the dimensions and data type of a dataset.
   */
  typedef struct CXI_Dataset{
    /*! The HDF5 identifier for the dataset.
     */
    hid_t handle;
    /*! The dimensions of the dataset or NULL if not set. */
    hsize_t * dimensions;
    /*! The number of dimensions of the dataset or 0 if not set. */
    int dimension_count;
    /*! The HDF5 data type of the element of the dataset, or 0 if not set. */
    hid_t data_type;    
  }CXI_Dataset;

  /*! A reference to an open \p CXI_Dataset
   */
  typedef struct CXI_Dataset_Reference{
    /*! The HDF5 identifier of the parent of the group. 
     */
    hid_t parent_handle;
    /*! The name of this group, e.g. "Dataset_1".      
     */
    char * group_name;
    /*! The \p CXI_Dataset to which this reference corresponds to. */
    CXI_Dataset * dataset;
  }CXI_Dataset_Reference;


  /*! Document an event of data processing, reconstruction, or analysis
   */
  typedef struct CXI_Process{
    /*! The HDF5 identifier for the process group.
     */
    hid_t handle;
  }CXI_Process;

  /*! A reference to an open \p CXI_Process
   */
  typedef struct CXI_Process_Reference{
    /*! The HDF5 identifier of the parent of the group. 
     */
    hid_t parent_handle;
    /*! The name of this group, e.g. "Process_1".      
     */
    char * group_name;
    /*! The \p CXI_Process to which this reference corresponds to. */
    CXI_Process * process;
  }CXI_Process_Reference;
  
  /*! Describes a beamline attenuator used during data collection.
   */
  typedef struct CXI_Attenuator{
    /*! The HDF5 identifier for the attenuator group.
     */
    hid_t handle;

    /*! Distance from sample.      
     */
    double distance;
    /*! Is 1 if \p distance is set and 0 if not.
     */
    int distance_valid;

    /*! Thickness of attenuator along beam direction. 
     */
    double thickness;
    /*! Is 1 if \p thickness is set and 0 if not.
     */
    int thickness_valid;
    
    /*! The nominal amount of the beam that gets through (transmitted intensity)/(incident intensity). */
    double attenuator_transmission;
    /*! Is 1 if \p attenuator_transmission is set and 0 if not.
     */
    int attenuator_transmission_valid;
    
    /*! Type or composition of attenuator, or NULL if not set. */
    char * type;
  }CXI_Attenuator;

  /*! A reference to an open \p CXI_Attenuator.
   */
  typedef struct CXI_Attenuator_Reference{
    /*! The HDF5 identifier of the parent of the group. 
     */
    hid_t parent_handle;
    /*! The name of this group, e.g. "Attenuator_1".      
     */
    char * group_name;
    /*! The \p CXI_Attenuator to which this reference corresponds to. */
    CXI_Attenuator * attenuator;
  }CXI_Attenuator_Reference;


  /*! Holds the general position and orientation of a component.
   */
  typedef struct CXI_Geometry{
    /*! The HDF5 identifier for the geometry group.
     */
    hid_t handle;
    /*! Dot products between the local and the global unit vectors. 
       Check the Geometry entry in the CXI File Format reference for further details. 
    */
    double orientation[2][3];
    /*! Is 1 if \p orientation is set and 0 if not. */
    int orientation_valid;
    
    /*! The x, y and z components of the translation of the origin of the
     *  object relative to the origin of the global coordinate system (the 
     *  place where the X-ray beam meets the sample).
     */
    double translation[3];
    /*! Is 1 if \p translation is set and 0 if not. */    
    int translation_valid;
  }CXI_Geometry;

  /*! A reference to an open \p CXI_Geomtry.
   */
  typedef struct CXI_Geometry_Reference{
    /*! The HDF5 identifier of the parent of the group. 
     */
    hid_t parent_handle;
    /*! The name of this group, e.g. "Geometry_1".      
     */
    char * group_name;
    /*! The \p CXI_Geometry to which this reference corresponds to. */
    CXI_Geometry * geometry;
  }CXI_Geometry_Reference;

  /*! Holds information about one of the detectors used during the experiment.
   *  Raw data recorded by a detector as well as its position and geometry
   *  should be stored in this class.
   */
  typedef struct CXI_Detector{
    /*! The HDF5 identifier for the detector group.
     */
    hid_t handle;

    /*! A matrix with the basis vectors of the detector data.
     *  For more details see the CXI File Format reference 8.2.1.
     */
    double basis_vectors[2][3];
    /*! Is 1 if \p basis_vectors is set and 0 if not. */
    int basis_vectors_valid;

    /*! The x, y and z coordinates of the corner of the first data element. */
    double corner_position[3];
    /*! Is 1 if \p corner_position is set and 0 if not. */
    int corner_position_valid;

    /*! Number of counts recorded per each joule of energy received by the detector. */
    double counts_per_joule;
    /*! Is 1 if \p counts_per_joule is set and 0 if not. */
    int counts_per_joule_valid;

    /*!  A reference to a dataset with recorded signal values,
      or NULL if not set. */
    CXI_Dataset_Reference * data;
    /*!  A reference to a dataset of the image recorded with the
      shutter closed, used for background subtractions value of
      the image at each pixel, or NULL if not set. */    
    CXI_Dataset_Reference * data_dark;
    /*! A reference to a dataset of the image recorded without
        the sample, used for background subtractions,
	or NULL if not set. */    
    CXI_Dataset_Reference * data_white;
    /*! A reference to a dataset of the the best estimate of the uncertainty
      in the data value. Where possible, this should be the standard deviation,
      which has the same units as the data. Or NULL if not set. */
    CXI_Dataset_Reference * data_error;

    /*! Sum of all the elements in the data array. 
      This number is often userful as a cheap measure of data quality. */
    double data_sum;
    /*! Is 1 if \p data_sum is set and 0 if not. */
    int data_sum_valid;

    /*! name/manufacturer/model/etc. information, or NULL if not set. */
    char * description;

    /*! Closest distance from the detector to the sample. */
    double distance;
    /*! Is 1 if \p distance is set and 0 if not. */
    int distance_valid;

    /*! Position and orientation of the center of mass of the detector.
       This should only be specified for non pixel detectors. For pixel
       detectors use basis vectors and corner position. Or NULL if not set. */
    CXI_Geometry_Reference * geometry;

    /*! Not all the pixels in a detector might have the same value.
       This 32bit mask makes it possible to distinguish different kinds of pixels.
       The following list defines the meaning of each bit when is
       it set, as well as the names of constants, defined in cxi.h.
       Or NULL if not set.
    */
    CXI_Dataset_Reference * mask;

    /*! Width of each detector pixel. */
    double x_pixel_size;
    /*! Is 1 if \p x_pixel_size is set and 0 if not. */
    int x_pixel_size_valid;

    /*! Height of each detector pixel. */
    double y_pixel_size;
    /*! Is 1 if \p y_pixel_size is set and 0 if not. */
    int y_pixel_size_valid;
  }CXI_Detector;

  /*! A reference to an open \p CXI_Detector.
   */
  typedef struct CXI_Detector_Reference{
    /*! The HDF5 identifier of the parent of the group. 
     */
    hid_t parent_handle;
    /*! The name of this group, e.g. "Detector_1".      
     */
    char * group_name;
    /*! The \p CXI_Detector to which this reference corresponds to. */
    CXI_Detector * detector;
  }CXI_Detector_Reference;

  /*! Describes the light source being used. */
  typedef struct CXI_Source{
    /*! The HDF5 identifier for the source group.
     */
    hid_t handle;

    /*! in Joules per photon! */
    double energy;
    /*! Is 1 if \p energy is set and 0 if not. */
    int energy_valid;

    /*! The name of the source, for example ALS. Or NULL if not set. */
    char * name;
    /*! in Joules per pulse! */
    double pulse_energy;
    /*! Is 1 if \p pulse_energy is set and 0 if not. */
    int pulse_energy_valid;

    /*! Pulse width in seconds */
    double pulse_width;
    /*! Is 1 if \p pulse_width is set and 0 if not. */
    int pulse_width_valid;
  }CXI_Source;


  /*! A reference to an open CXI_Source.
   */
  typedef struct CXI_Source_Reference{
    /*! The HDF5 identifier of the parent of the group. 
     */
    hid_t parent_handle;
    /*! The name of this group, e.g. "Source_1".      
     */
    char * group_name;
    /*! The \p CXI_Source to which this reference corresponds to. */
    CXI_Source * source;
  }CXI_Source_Reference;

  /*! Monochromator used in the instrument. */
  typedef struct CXI_Monochromator{
    /*! The HDF5 identifier for the monochromator group.
     */
    hid_t handle;

    /*! Peak of the spectrum that the monochromator selects. */
    double energy;
    /*! Is 1 if \p energy is set and 0 if not. */
    int energy_valid;

    /*! Standard deviation of the spectrum that the monochromator selects.*/
    double energy_error;
    /*! Is 1 if \p energy_error is set and 0 if not. */
    int energy_error_valid;
  }CXI_Monochromator;
  
  /*! A reference to an open CXI Monochromator.
   */
  typedef struct CXI_Monochromator_Reference{
    /*! The HDF5 identifier of the parent of the group. 
     */
    hid_t parent_handle;
    /*! The name of this group, e.g. "Monochromator_1".      
     */
    char * group_name;
    /*! The \p CXI_Monochromator to which this reference corresponds to. */
    CXI_Monochromator * monochromator;
  }CXI_Monochromator_Reference;
  
  /*! Template of instrument descriptions comprising various beamline components.
   *  Each component will also be a class defined by its distance from the sample.
   *  Negative distances represent beamline components that are before the sample
   *  while positive distances represent components that are after the sample.
   *  This device allows the unique identification of beamline components in a way
   *  that is valid for both reactor and pulsed instrumentation.
   *  Each Instrument instance corresponds to one beamline.
   */
  typedef struct CXI_Instrument{
    /*! The HDF5 identifier for the instrument group.
     */
    hid_t handle;

    /*! Name of the instrument or NULL if not set */
    char * name;
    /*! The attenuators that are part of the instrument. */
    CXI_Attenuator_Reference ** attenuators;
    /*! The number of CXI attenuator groups in the list.
      \see attenuators
     */
    int attenuator_count;
    /*! The detectors that compose the instrument. */
    CXI_Detector_Reference ** detectors;
    /*! The number of CXI detector groups in the list.
      \see detectors
     */
    int detector_count;
    /*! The monochromators that compose the instrument. */
    CXI_Monochromator_Reference ** monochromators;
    /*! The number of CXI monochromator groups in the list.
      \see monochromators
     */
    int monochromator_count;
    /*! The source used by the instrument. */
    CXI_Source_Reference ** sources;
    /*! The number of CXI source groups in the list.
      \see sources
     */
    int source_count;
  }CXI_Instrument;

  /*! A reference to an open CXI_Instrument.
   */
  typedef struct CXI_Instrument_Reference{
    /*! The HDF5 identifier of the parent of the group. 
     */
    hid_t parent_handle;
    /*! The name of this group, e.g. "Instrument_1".      
     */
    char * group_name;
    /*! The \p CXI_Instrument to which this reference corresponds to. */
    CXI_Instrument * instrument;
  }CXI_Instrument_Reference;

  /*! This class is a general placeholder for the most important information 
   *  in each Entry class. It is mandatory that there is at least one Data class
   *  in each Entry class. Most data analysis and plotting programs will primarily 
   *  focus in this class.
   */
  typedef struct CXI_Data{
    /*! The HDF5 identifier for the data group.
     */
    hid_t handle;
    /*! Most important data values or NULL if not set. */
    CXI_Dataset_Reference * data;
    /*! Standard deviations of data values or NULL if not set. */
    CXI_Dataset_Reference * errors;    
  }CXI_Data;

  /*! A reference to an open CXI_Data.
   */
  typedef struct CXI_Data_Reference{
    /*! The HDF5 identifier of the parent of the group. 
     */
    hid_t parent_handle;
    /*! The name of this group, e.g. "Data_1".      
     */
    char * group_name;
    /*! The \p CXI_Data to which this reference corresponds to. */
    CXI_Data * data;
  }CXI_Data_Reference;

  /*! This class should be used to store processed image data. 
   *  It describes what analysis has been done, as well as holding
   *  important information for further image processing. It should
   *  not be used for raw data storage, which should be stored in
   *  the Detector class.
   */
  typedef struct CXI_Image{
    /*! The HDF5 identifier for the image group.
     */
    hid_t handle;
    /*! The value of the image at each pixel or NULL if not set. */
    CXI_Dataset_Reference * data;
    /*! The best estimate of the uncertainty in the data value. Where possible, this
      should be the standard deviation, which has the same units as the data. Or NULL if not set. */
    CXI_Dataset_Reference * data_error;
    /*! Specifies if the image lives in real or diffraction (Fourier) space.
      CXI_Invalid_Space is not set.
     \see CXI_Image_Space 
    */
    int data_space;
    /*! Defines what the data represents. 
     \see CXI_Image_Type */
    char * data_type;

    /*! List to the detectors used to obtain this image. Or NULL if not set. */
    CXI_Detector_Reference ** detectors;
    /*! The number of CXI detector groups in the list.
     */
    int detector_count;

    /*! Number of dimensions of the image. Restricted to 1 2 or 3. */
    int dimensionality;
    /*! Is 1 if \p dimensionality is set and 0 if not. */
    int dimensionality_valid;

    /*! The location of the zero frequency component on a diffraction image in fractional pixels. 
      The top left corner of the top left pixel has the coordinates (0,0). The bottom right corner of the
      top left pixel has the coordinates (1,1).
      See the CXI File format reference for an image of the coordinate system convention. */
    double image_center[3];
    /*! Is 1 if \p image_center is set and 0 if not. */
    int image_center_valid;

    /*! The width, height and depth of the image. For real space images this
      corresponds to the length and for reciprocal space ones to the inverse
      length of the sides of the image. */
    double image_size[3];
    /*! Is 1 if \p image_size is set and 0 if not. */
    int image_size_valid;

    /*! If set to 1 the image is assumed to have to the quadrants shifted. */
    int is_fft_shifted;
    /*! Is 1 if \p is_fft_shifted is set and 0 if not. */
    int is_fft_shifted_valid;

    /*!  32-bit unsigned integer mask specifying the properties of each
      pixel. The following list defines the meaning of each bit when is it set, as well as the names of constants, defined in cxi.h, useful for checkings their values: 
      | Bit        | Meaning                                                  | Constant                 |
      | :--------: | :------------------------------------------------------: | :----------------------: |
      | 0x00000001 | If set the pixel is valid                                | CXI PIXEL IS VALID       |
      | 0x00000002 | If set the pixel is saturated                            | CXI PIXEL IS SATURATED   |
      | 0x00000004 | If set the pixel is hot                                  | CXI PIXEL IS HOT         |
      | 0x00000008 | If set the pixel is dead                                 | CXI PIXEL IS DEAD        |
      | 0x00000010 | If set the pixel is under a shadow                       | CXI PIXEL IS SHADOWED    |
      | 0x00000020 | If set the pixel is iluminated by parasitic light        | CXI PIXEL IS PARASITIC   |
      | 0x00000040 | If set the pixel signal is above the background          | CXI PIXEL HAS SIGNAL     |
      | 0x00000200 | If set the pixel is inside of the reconstruction support | CXI PIXEL INSIDE SUPPORT |

      All other bits have no standard meaning and can be used for any purpose the user sees fit. More bits will be defined as the format evolves so users are encouranged to use the high bits to avoid collisions.
    */
    CXI_Dataset_Reference * mask;

    /*! List of processes used to obtain this image. They should be listed in chronological order with the first processed used first and so on.*/
    CXI_Process_Reference ** processs;
    /*! The number of CXI process groups in the list.
     */
    int process_count;

    /*! The diffraction (Fourier) space coordinates of the center of each pixel.
      Note that the dimension corresponding to the 3 different components should go before the image dimensions.
      So for example for an image of size [10,20,5] te reciprocal coordinates will have size [3,10,20,5].
      Or NULL if not set.
    */
    CXI_Dataset_Reference * reciprocal_coordinates;

    /*! The source used to obtain this image. */
    CXI_Source_Reference ** sources;
    /*! The number of CXI source groups in the list.
     */
    int source_count;    
  }CXI_Image;
  
  /*! A reference to an open CXI_Image.
   */
  typedef struct CXI_Image_Reference{
    /*! The HDF5 identifier of the parent of the group. 
     */
    hid_t parent_handle;
    /*! The name of this group, e.g. "Image_1".      
     */
    char * group_name;
    /*! The \p CXI_Image to which this reference corresponds to */
    CXI_Image * image;
  }CXI_Image_Reference;
  
  /*! Holds basic information about the kind of sample used, its geometry and properties.
   */
  typedef struct CXI_Sample{
    /*! The HDF5 identifier for the sample group.
     */
    hid_t handle;

    /*! Concentration of the sample in kg/m^3 */
    float concentration;
    /*! Is 1 if \p concentration is set and 0 if not. */
    int concentration_valid;

    /*! Description of the sample. */
    char * description;
    
    /*! Position and orientation of the center of mass of the sample. */
    CXI_Geometry_Reference * geometry;
    
    /*! Mass of sample. */
    float mass;
    /*! Is 1 if \p mass is set and 0 if not. */
    int mass_valid;

    /*! Descriptive name of sample. */
    char * name;
    
    /*! Sample temperature, in kelvin. */
    float temperature;
    /*! Is 1 if \p temperature is set and 0 if not. */
    int temperature_valid;
    
    /*! Unit cell parameters (a,b,c α, β, γ).*/
    float unit_cell[2][3];
    /*! Is 1 if \p unit_cell is set and 0 if not. */
    int unit_cell_valid;
    
    /*! Crystallographic space group of the crystal in PDB format. */
    char * unit_cell_group;

    /*! Sample thickness. */
    float thickness;
    /*! Is 1 if \p thickness is set and 0 if not. */
    int thickness_valid;
    
    /*! Volume of the unit cell. */
    float unit_cell_volume;
    /*! Is 1 if \p unit_cell_volume is set and 0 if not. */
    int unit_cell_volume_valid;
  }CXI_Sample;

  /*! A reference to an open CXI_Sample.
   */
  typedef struct CXI_Sample_Reference{
    /*! The HDF5 identifier of the parent of the group. 
     */
    hid_t parent_handle;
    /*! The name of this group, e.g. "Sample_1".      
     */
    char * group_name;
    /*! The \p CXI_Sample to which this reference corresponds to */
    CXI_Sample * sample;
  }CXI_Sample_Reference;

  /*! Describes the properties of a CXI Entry.
   *  Base CXI class which holds all other classes.
   */
  typedef struct CXI_Entry{
    /*! The HDF5 identifier of the CXI Entry. 
     */
    hid_t handle;
    /*! A list of the CXI data groups under this entry.
     */
    CXI_Data_Reference ** data;
    /*! The number of CXI data groups in the list.
     */
    int data_count;

    /*! The ending date and time of the measurement in ISO8601 format,
     *  following the CXI specification.
     *  For example "2009-05-21T15:12:03+0900".
     */
    char * end_time;

    /*! A free form identifier. Unique identifier for the experiment, 
     *  defined by the facility, possibly linked to the proposals.
     */
    char * experiment_identifier;
    /*! A description of the experiment.
     */
    char * experiment_description;

    /*! A list of the CXI image groups under this entry.
     */
    CXI_Image_Reference ** images;
    /*! The number of CXI image groups in the list.
     */
    int image_count;

    /*! A list of the CXI instrument groups under this entry.
     */
    CXI_Instrument_Reference ** instruments;
    /*! The number of CXI instrument groups in the list.
     */
    int instrument_count;

    /*! Name of program used to generate this file.
     */
    char * program_name;

    /*! A list of the CXI sample groups under this entry.
     */
    CXI_Sample_Reference ** samples;
    /*! The number of CXI sample groups in the list.
     */
    int sample_count;

    /*! The starting date and time of the measurement in ISO8601 format,
     *  following the CXI specification.
     *  For example "2009-05-21T15:12:03+0900".
     */
    char * start_time;

    /*! The title of the entry 
     */
    char * title;
  }CXI_Entry;

  /*! A reference to an open CXI_Entry.
   */
  typedef struct CXI_Entry_Reference{
    /*! The HDF5 identifier of the parent of the entry. 
     */
    hid_t parent_handle;
    /*! The name of this entry, e.g. "Entry_1".      
     */
    char * group_name;
    /*! The CXI_Entry to which this reference corresponds to. */
    CXI_Entry * entry;
  }CXI_Entry_Reference;


  /*! Contains information about an open CXI file.
   */
  typedef struct CXI_File{
    /*! The HDF5 identifier for the file.
     */
    hid_t handle;
    /*! The name of the file. Might include the path.
     */
    char * filename;
    /*! A list of the CXI entries in the HDF5 root of this file.
     */
    CXI_Entry_Reference ** entries;
    /*! The number of CXI entries in the HDF5 root of this file.
     */
    int entry_count;
    /*! The version of the CXI file as an integer. 
     * This corresponds to 100x the typical version number. 
     * For example 130 corresponds to 1.30. 
     * A negative value indicates the value was not set/read.
     */
    int cxi_version;
  }CXI_File;


  /*! Open or create a new CXI file.
   * 
   * \param filename The name of the file.
   * \param mode The mode in which to open or create the file.
   * \p mode can be one of two possibilities:
   * - "r" - Open an existing file in read-only mode.
   * - "w" - Create a new file. If a file exists with the same name it will be truncated (deleted).
   *
   * \return The opened/created \p CXI_File or NULL in case of error.
   */
  CXI_File * cxi_open_file(const char * filename, const char * mode);

  /*! Close an open CXI file. 
   *
   * \param file The file to close.
   * \return Zero if successful ora negative number in case of error.
   */
  int cxi_close_file(CXI_File * file);

  /*! Open a CXI Entry 
   *
   * \param entry A reference to the entry to be opened.
   *
   * \return The opened \p CXI_Entry or NULL is case of error.
   */
  CXI_Entry * cxi_open_entry(CXI_Entry_Reference * entry);

  /*! Open a CXI Instrument 
   *
   * \param instrument A reference to the instrument to be opened.
   *
   * \return The opened \p CXI_Instrument or NULL is case of error.
   */
  CXI_Instrument * cxi_open_instrument(CXI_Instrument_Reference * instrument);

  /*! Open a CXI Detector 
   *
   * \param detector A reference to the detector to be opened.
   *
   * \return The opened \p CXI_Detector or NULL is case of error.
   */
  CXI_Detector * cxi_open_detector(CXI_Detector_Reference * detector);
 
  /*! Open a CXI Attenuator 
   *
   * \param attenuator A reference to the attenuator to be opened.
   *
   * \return The opened \p CXI_Attenuator or NULL is case of error.
   */
  CXI_Attenuator * cxi_open_attenuator(CXI_Attenuator_Reference * attenuator);

  /*! Open a CXI Monochromator 
   *
   * \param monochromator A reference to the monochromator to be opened.
   *
   * \return The opened \p CXI_Monochromator or NULL is case of error.
   */
  CXI_Monochromator * cxi_open_monochromator(CXI_Monochromator_Reference * monochromator);
  /*! Open a CXI Sample
   *
   * \param sample A reference to the sample to be opened.
   *
   * \return The opened \p CXI_Sample or NULL is case of error.
   */
  CXI_Sample * cxi_open_sample(CXI_Sample_Reference * sample);
  /*! Open a CXI Source
   *
   * \param source A reference to the source to be opened.
   *
   * \return The opened \p CXI_Source or NULL is case of error.
   */
  CXI_Source * cxi_open_source(CXI_Source_Reference * source);
  /*! Open a CXI Data
   *
   * \param data A reference to the data to be opened.
   *
   * \return The opened \p CXI_Data or NULL is case of error.
   */
  CXI_Data * cxi_open_data(CXI_Data_Reference * data);
  /*! Open a CXI Dataset
   *
   * \param dataset A reference to the dataset to be opened.
   *
   * \return The opened \p CXI_Dataset or NULL is case of error.
   */
  CXI_Dataset * cxi_open_dataset(CXI_Dataset_Reference * dataset);
  

  /*! Read an open a CXI Dataset
   *
   * \param dataset The dataset to read.
   * \param data The buffer where the read data will be written.
   * \param data_type The HDF5 data type to be written on the output buffer. Must be convertible from the data type of the dataset.
   *
   * \return Zero if successful or a negative number in case of error.
   */
  int cxi_read_dataset(CXI_Dataset * dataset, void * data, hid_t data_type);

  /*! Read a slice from an open a CXI Dataset
   *
   * \param dataset The dataset to read.
   * \param slice The index of the slice to read.
   * \param data The buffer where the read data will be written.
   * \param data_type The HDF5 data type to be written on the output buffer. Must be convertible from the data type of the dataset.
   *
   * \return Zero if successful or a negative number in case of error.
   */
  int cxi_read_dataset_slice(CXI_Dataset * dataset, unsigned int slice, void * data, hid_t data_type);

  /*! Write a entry group.
   * 
   * \param loc An HDF5 identifier specifying the location where the dataset will be created.
   * \param entry A filled structure which determines the properties of the created entry.
   *
   * \return A reference to the \p entry created or NULL in case of error.
   */
  CXI_Entry_Reference * cxi_write_entry(hid_t loc, CXI_Entry * entry);
  /*! Write a data group.
   * 
   * \param loc An HDF5 identifier specifying the location where the dataset will be created.
   * \param data A filled structure which determines the properties of the created data.
   *
   * \return A reference to the \p data created or NULL in case of error.
   */
  CXI_Data_Reference * cxi_write_data(hid_t loc, CXI_Data * data);
  /*! Write a image group.
   * 
   * \param loc An HDF5 identifier specifying the location where the dataset will be created.
   * \param image A filled structure which determines the properties of the created image.
   *
   * \return A reference to the \p image created or NULL in case of error.
   */
  CXI_Image_Reference * cxi_write_image(hid_t loc, CXI_Image * image);
  /*! Write a instrument group.
   * 
   * \param loc An HDF5 identifier specifying the location where the dataset will be created.
   * \param instrument A filled structure which determines the properties of the created instrument.
   *
   * \return A reference to the \p instrument created or NULL in case of error.
   */
  CXI_Instrument_Reference * cxi_write_instrument(hid_t loc, CXI_Instrument * instrument);
  /*! Write a sample group.
   * 
   * \param loc An HDF5 identifier specifying the location where the dataset will be created.
   * \param sample A filled structure which determines the properties of the created sample.
   *
   * \return A reference to the \p sample created or NULL in case of error.
   */
  CXI_Sample_Reference * cxi_write_sample(hid_t loc, CXI_Sample * sample);
  /*! Write a attenuator group.
   * 
   * \param loc An HDF5 identifier specifying the location where the dataset will be created.
   * \param attenuator A filled structure which determines the properties of the created attenuator.
   *
   * \return A reference to the \p attenuator created or NULL in case of error.
   */
  CXI_Attenuator_Reference * cxi_write_attenuator(hid_t loc, CXI_Attenuator * attenuator);
  /*! Write a monochromator group.
   * 
   * \param loc An HDF5 identifier specifying the location where the dataset will be created.
   * \param monochromator A filled structure which determines the properties of the created monochromator.
   *
   * \return A reference to the \p monochromator created or NULL in case of error.
   */
  CXI_Monochromator_Reference * cxi_write_monochromator(hid_t loc, CXI_Monochromator * monochromator);
  /*! Write a source group.
   * 
   * \param loc An HDF5 identifier specifying the location where the dataset will be created.
   * \param source A filled structure which determines the properties of the created source.
   *
   * \return A reference to the \p source created or NULL in case of error.
   */
  CXI_Source_Reference * cxi_write_source(hid_t loc, CXI_Source * source);
  /*! Write a process group.
   * 
   * \param loc An HDF5 identifier specifying the location where the dataset will be created.
   * \param process A filled structure which determines the properties of the created process.
   *
   * \return A reference to the \p process created or NULL in case of error.
   */
  CXI_Process_Reference * cxi_write_process(hid_t loc, CXI_Process * process);

  /*! Create a new dataset.
   * 
   * \param loc An HDF5 identifier specifying the location where the dataset will be created.
   * \param dataset A filled structure which determines the properties of the created dataset.
   * \param type The type of dataset created.
   *
   * \return A reference to the \p dataset created or NULL in case of error.
   *
   * The following snippet shows how to create a dataset for 10 integers:
   * \code
   
    #include <cxi.h>
    ...
    CXI_Detector * det;
    ...
    CXI_Dataset dataset;
    dataset.dimension_count = 1;
    dataset.dimensions = malloc(sizeof(hsize_t)*1);
    dataset.dimensions[0] = 10;
    dataset.data_size = dataset.dimensions[0]*sizeof(int);
    dataset.data_type = H5T_NATIVE_INT;
   
    cxi_create_dataset(det->handle, dataset, CXI_Data_Type);
    ...
   
    \endcode
   *
   */
   
  CXI_Dataset_Reference * cxi_create_dataset(hid_t loc, CXI_Dataset * dataset, 
					     CXI_Dataset_Type type);

  /*! Write data to a dataset.
   * 
   * \param dataset The \p dataset to write to.
   * \param data The \p data to be written.
   * \param data_type The HDF5 type of the elements of the data. It has to be
   * convertible to the data_type of the \p dataset.
   *
   * \return Zero if succesful or non-zero if it encountered an error.
   *
   * The following snippet shows how to write 10 integers to a dataset.
   * \code
   
    #include <cxi.h>
    ...
    CXI_Dataset dataset;
    ...
    int data[10] = {0,1,2,3,4,5,6,7,8,9};
    cxi_write_dataset(&dataset, data, H5T_NATIVE_INT);
    ...
   
    \endcode
   *
   */
  int cxi_write_dataset(CXI_Dataset * dataset, void * data, hid_t data_type);

  /*! Write a slice of data to a dataset.
   * 
   * \param dataset The \p dataset to write to.
   * \param slice The 0-based index of the slice to be written.
   * \param data The \p data to be written.
   * \param data_type The HDF5 type of the elements of the data. It has to be
   * convertible to the data_type of the \p dataset.
   *
   * \return Zero if succesful or non-zero if it encountered an error.
   *
   * The following snippet shows how to write a single integer to a dataset of 10 integers.
   * \code
   
    #include <cxi.h>
    ...
    CXI_Dataset dataset;
    ...
    int data = 3;
    cxi_write_dataset_slice(&dataset, 3, &data, H5T_NATIVE_INT);
    ...
   
    \endcode
   *
   */
  int cxi_write_dataset_slice(CXI_Dataset * dataset, unsigned int slice, void * data, hid_t data_type);


  /*! Calculate the total number of elements in a dataset
   *
   * \param dataset The \p dataset used to calculate the size.
   *
   * \return The total number of element in the dataset.
   */
  hsize_t cxi_dataset_length(CXI_Dataset * dataset);

#ifdef __cplusplus 
} /* extern "C" */
#endif  

