#include <stdlib.h>
#include <string.h>

#include "board_controller.h"
#include "board_shim.h"

BoardShim::BoardShim (int board_id, const char *port_name)
{
    strcpy (this->port_name, port_name);
    this->board_id = board_id;
    if (board_id == CYTON_BOARD)
    {
        board_desc = new CytonDesc ();
    }
    else
    {
        if (board_id == GANGLION_BOARD)
        {
            board_desc = new GanglionDesc ();
        }
        else
        {
            board_desc = NULL;
            throw BrainFlowExcpetion ("Unsupported Board Error", UNSUPPORTED_BOARD_ERROR);
        }
    }
}

void BoardShim::prepare_session ()
{
    int res = ::prepare_session (board_id, port_name);
    if (res != STATUS_OK)
    {
        throw BrainFlowExcpetion ("failed to prepare session", res);
    }
}

void BoardShim::start_stream (int buffer_size)
{
    int res = ::start_stream (buffer_size);
    if (res != STATUS_OK)
    {
        throw BrainFlowExcpetion ("failed to start stream", res);
    }
}

void BoardShim::stop_stream ()
{
    int res = ::stop_stream ();
    if (res != STATUS_OK)
    {
        throw BrainFlowExcpetion ("failed to stop stream", res);
    }
}

void BoardShim::release_session ()
{
    int res = ::release_session ();
    if (res != STATUS_OK)
    {
        throw BrainFlowExcpetion ("failed to release session", res);
    }
}

void BoardShim::get_board_data_count (int *result)
{
    int res = ::get_board_data_count (result);
    if (res != STATUS_OK)
    {
        throw BrainFlowExcpetion ("failed to get board data count", res);
    }
}

void BoardShim::get_board_data (int data_count, double **data_buf)
{
    float *buf = new float[data_count * board_desc->get_num_data_channels ()];
    double *ts_buf = new double[data_count];

    int res = ::get_board_data (data_count, buf, ts_buf);
    if (res != STATUS_OK)
    {
        delete[] buf;
        delete[] ts_buf;
        throw BrainFlowExcpetion ("failed to get board data", res);
    }
    reshape_data (data_count, buf, ts_buf, data_buf);
    delete[] buf;
    delete[] ts_buf;
}

void BoardShim::get_current_board_data (int num_samples, double **data_buf, int *returned_samples)
{
    float *buf = new float[num_samples * board_desc->get_num_data_channels ()];
    double *ts_buf = new double[num_samples];

    int res = ::get_current_board_data (num_samples, buf, ts_buf, returned_samples);
    if (res != STATUS_OK)
    {
        delete[] buf;
        delete[] ts_buf;
        throw BrainFlowExcpetion ("failed to get board data", res);
    }
    reshape_data (*returned_samples, buf, ts_buf, data_buf);
    delete[] buf;
    delete[] ts_buf;
}

void BoardShim::reshape_data (int data_count, float *data_buf, double *ts_buf, double **output_buf)
{
    for (int i = 0; i < data_count; i++)
    {
        for (int j = 0; j < board_desc->get_num_data_channels (); j++)
        {
            output_buf[i][j] = data_buf[i * board_desc->get_num_data_channels () + j];
        }
        output_buf[i][board_desc->get_num_data_channels ()] = ts_buf[i];
    }
}
