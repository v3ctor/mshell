#ifndef _V3CT0R_PIPELINE_H
#define _V3CT0R_PIPELINE_H

#include <stdbool.h>

#include "io.h"
#include "siparse.h"

bool is_valid_pipelineseq(pipelineseq *ps);
void execute_pipeline_sequence(std_io *io, pipelineseq *seq);
void pipeline_init(void);
void print_bg_notes(std_io *io);
void clear_bg_notes(void);

#endif // _V3ECT0R_ERRORS_H
