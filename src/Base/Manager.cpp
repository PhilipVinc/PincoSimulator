//
//  Manager.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 18/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "Manager.hpp"

#include "DataSaver.hpp"
#include "Settings.hpp"
#include "Task.hpp"

Manager::Manager(const Settings* _settings)
{
    settings = _settings;
    saver = new DataSaver(settings);
}

Manager::~Manager()
{
    
}

void Manager::SaveTask(Task* task)
{
    saver->SaveData(task->GetResults());
}

