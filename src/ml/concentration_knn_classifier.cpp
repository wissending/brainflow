#include <algorithm>
#include <cmath>
#include <stdlib.h>

#include "brainflow_constants.h"
#include "concentration_knn_classifier.h"
#include "focus_dataset.h"


int ConcentrationKNNClassifier::prepare ()
{
    if (kdtree != NULL)
    {
        return (int)BrainFlowExitCodes::ANOTHER_CLASSIFIER_IS_PREPARED_ERROR;
    }
    if (!params.other_info.empty ())
    {
        try
        {
            num_neighbors = std::stoi (params.other_info);
        }
        catch (const std::exception &e)
        {
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }
    if ((num_neighbors < 1) || (num_neighbors > 100))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    int dataset_len = sizeof (brainflow_focus_y) / sizeof (brainflow_focus_y[0]);
    for (int i = 0; i < dataset_len; i++)
    {
        FocusPoint point (brainflow_focus_x[i], 10, brainflow_focus_y[i]);
        // decrease weight for stddev, 0.2 - experimental vlaue
        for (int j = 5; j < 10; j++)
        {
            point[j] *= 0.2;
        }
        dataset.push_back (point);
    }
    kdtree = new kdt::KDTree<FocusPoint> (dataset);
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationKNNClassifier::predict (double *data, int data_len, double *output)
{
    if ((data_len < 5) || (data == NULL) || (output == NULL))
    {
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    if (kdtree == NULL)
    {
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }

    double feature_vector[10] = {0.0};
    for (int i = 0; i < data_len; i++)
    {
        if (i >= 5)
        {
            feature_vector[i] = data[i] * 0.2;
        }
        else
        {
            feature_vector[i] = data[i];
        }
    }

    FocusPoint sample_to_predict (feature_vector, 10, 0.0);
    const std::vector<int> knn_ids = kdtree->knnSearch (sample_to_predict, num_neighbors);
    int num_ones = 0;
    for (int i = 0; i < knn_ids.size (); i++)
    {
        if (dataset[knn_ids[i]].value == 1)
        {
            num_ones++;
        }
    }

    double score = ((double)num_ones) / num_neighbors;
    *output = score;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationKNNClassifier::release ()
{
    delete kdtree;
    kdtree = NULL;
    dataset.clear ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}
