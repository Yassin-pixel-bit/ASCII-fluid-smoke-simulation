#include "settings.h"

using namespace std;

const string ENGINE_HEADER = "Engine";
const string SIM_HEADER = "Simulation Values";
const string FLUID_HEADER = "fluid Settings";

void init_ini()
{
    // create the file manually so I can put comments
    ofstream out_file("settings.ini");

    if (!out_file.is_open())
        throw runtime_error("Error: Failed to generate or write to the configuration file.");

    // Write the Engine section
    out_file << "[" << ENGINE_HEADER << "]\n";
    out_file << "fps = 165\n\n";

    // Write the Simulation Values section
    out_file << "[" << SIM_HEADER << "]\n";
    out_file << "wind_force = 500.0\n";
    out_file << "fluid_amount = 1000.0\n";
    out_file << "spawn_push = 500.0\n";
    out_file << "; values from 0.0 to 1.0\n";
    out_file << "spawn_x = 0.5\n";
    out_file << "spawn_y = 0.0\n\n";

    // Write the fluid Settings section
    out_file << "[" << FLUID_HEADER << "]\n";
    out_file << "; suggestion: keep the values below 1\n";
    out_file << "viscousity = 0.001\n";
    out_file << "diffusion = 0.00001\n";

    // TODO: add fluid spawn radius, wind radius, equal spawn distruption,  

    out_file.close();
}

bool check_section(mINI::INIStructure& ini, const std::string& section, std::vector<std::string>& warnings)
{
    if (!ini.has(section))
    {
        warnings.push_back("WARNING: Missing header [" + section + "]. Entire section will use defaults.");
        return false;
    }
    return true;
}

void load_float(mINI::INIStructure& ini, const string& section, const string& key, float& out_val, vector<std::string>& warnings)
{
    if (ini[section].has(key))
    {
        string raw_string = ini[section][key];
        try 
        {
            out_val = stof(raw_string);
        } 
        catch (const invalid_argument& e) 
        {
            warnings.push_back("WARNING: '" + raw_string + "' is not a valid number for [" + section + "][" + key + "]. Using default.");
        }
        catch (const out_of_range)
        {
            warnings.push_back("WARNING: The number '" + raw_string + "' is way too large/small for [" + section + "][" + key + "]. Using default.");
        }
    }
    else
    {
        warnings.push_back("WARNING: Missing key '" + key + "' in [" + section + "]. Using default.");
    }
}

void read_ini(sim_config& config, vector<string>& warnings)
{
    mINI::INIFile file("settings.ini");
    mINI::INIStructure ini;

    if (!file.read(ini))
        throw runtime_error("ERROR: Failed to read from the configuration file");

    // engine section
    if (check_section(ini, ENGINE_HEADER, warnings))
    {
        load_float(ini, ENGINE_HEADER, "fps", config.target_fps, warnings);
    }

    // fluid simulation section
    if (check_section(ini, SIM_HEADER, warnings))
    {
        load_float(ini, SIM_HEADER, "wind_force", config.wind_force, warnings);
        load_float(ini, SIM_HEADER, "fluid_amount", config.fluid_amount, warnings);
        config.fluid_amount = max(config.fluid_amount, 0.0f);
        load_float(ini, SIM_HEADER, "spawn_push", config.spawn_push, warnings);
        load_float(ini, SIM_HEADER, "spawn_x", config.spawn_x, warnings);
        config.spawn_x = clamp(config.spawn_x, 0.0f, 1.0f);
        load_float(ini, SIM_HEADER, "spawn_y", config.spawn_y, warnings);
        config.spawn_y = clamp(config.spawn_y, 0.0f, 1.0f);
    }

    // fluid settings
    if (check_section(ini, FLUID_HEADER, warnings))
    {
        load_float(ini, FLUID_HEADER, "viscousity", config.visc, warnings);
        config.visc = max(config.visc, 0.0f);
        load_float(ini, FLUID_HEADER, "diffusion", config.diff, warnings);
        config.diff = max(config.diff, 0.0f);
    } 
}

void get_user_settings(sim_config& config, vector<string>& warnings)
{
    bool needs_init = true;

    ifstream check_file("settings.ini");

    if (check_file.is_open() && check_file.peek() != ifstream::traits_type::eof())
    {
        needs_init = false; 
    }

    check_file.close();

    if (needs_init)
    {
        init_ini();
    }
    read_ini(config, warnings);
}