//
//  Simulation.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "Task.hpp"


Task::Task()
{
    task_id = 0;
}

Task::Task(size_t id)
{
    task_id = id;
}

Task::~Task()
{}

void Task::Execute()
{
    PreCompute();
    Compute();
    PostCompute();
}

void Task::PreCompute()
{}
void Task::PostCompute()
{}

void Task::Save()
{}

float Task::ApproximateComputationProgress()
{
    return 0;
}
