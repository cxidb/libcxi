#include <stdlib.h>
#include <string.h>
#include <cxi.h>

int main(int argc, char ** argv){
  if(argc < 2){
    printf("Usage: simple <cxi file>\n");
    return 0;
  }

  CXI_File * file = cxi_open_file(argv[1],"w");

  CXI_Entry * entry = calloc(sizeof(CXI_Entry),1);
  entry->end_time = strdup("2013-01-12T08:02:24+0100");
  entry->experiment_identifier = strdup("L730");
  entry->start_time = strdup("2013-01-12T08:00:00+0100");
  entry->title = strdup("Dummy entry");
  

  CXI_Entry_Reference * entry_ref = cxi_write_entry(file->handle,entry);
  if(!entry_ref){
    return -1;
  }


  return 0;
}
