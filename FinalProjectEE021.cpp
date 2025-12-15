#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <limits>
#include <iomanip>


using namespace std;


const double PI = 3.1415926535;


struct Signal {
    vector<double> data;
    string type;
    double frequency;
    double amplitude;
    double phase;
    double offset;
};


double get_double(const string& prompt) {
    double value;
    cout << prompt;
    while (!(cin >> value)) {
        cout << "Invalid input. Please enter a number: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return value;
}


void menu() {
    cout << "\n === Signal Generator + Circuit Menu === \n";
    cout << "1. Generate Sine Signal \n";
    cout << "2. Generate Square Signal \n";
    cout << "3. Generate Triangle Signal \n";
    cout << "4. Show Stored Signals \n";
    cout << "5. List Signal Properties (on RC output) \n";
    cout << "6. Load Signal File \n";
    cout << "7. Save Signal File \n";
    cout << "8. Simulate RC Circuit (use last raw signal) \n";
    cout << "9. Save RC Output Signal \n";
    cout << "10. 4D Array (time, I, V, R) Analysis \n";
    cout << "11. Save 4D Array \n";
    cout << "12. Exit \n";
}



void signal_properties_menu() {
    cout << "\n Signal Properties You Can Measure (RC output): \n";
    cout << "1. Peak-to-peak amplitude\n";
    cout << "2. Total energy (sum of v(t)^2 * dt)\n";
    cout << "3. RMS value\n";
    cout << "4. Numerical integration (trapezoidal area)\n";
}


//signal generation


vector<double> generate_signal(string type, double frequency,
                               double amplitude, double phase,
                               double offset, int points = 100) {
    vector<double> signal(points);
    for (int i = 0; i < points; ++i) {
        double time = i / (double)points;  
        double time_s = time * frequency + phase / (2 * PI);


        if (type == "sine")
            signal[i] = amplitude * sin(2 * PI * frequency * time + phase) + offset;
        else if (type == "square")
            signal[i] = amplitude * (sin(2 * PI * frequency * time + phase) >= 0 ? 1 : -1) + offset;
        else if (type == "triangle")
            signal[i] = amplitude * (2 * abs(2 * (time_s - floor(time_s + 0.5))) - 1) + offset;
    }
    return signal;
}


void display_signal(const vector<Signal>& signals) {
    if (signals.empty()) {
        cout << "No signals stored yet.\n";
        return;
    }
    for (size_t i = 0; i < signals.size(); ++i) {
        cout << "Signal " << i << " (" << signals[i].type << "): ";
        for (size_t j = 0; j < signals[i].data.size(); ++j)
            cout << signals[i].data[j] << " ";
        cout << endl;
    }
}


// Save
void save_signal(const vector<Signal>& signals) {
    if (signals.empty()) {
        cout << "No signals to save.\n";
        return;
    }
    
    int signal_index = signals.size();
    signal_index--;


    string filename;
    cout << "Enter filename (e.g., signal.csv): ";
    cin >> filename;
    
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Could not open file for writing.\n";
        return;
    }
    
    const Signal& sig = signals[signal_index];
    
    file << "# Signal Type: " << sig.type << "\n";
    file << "# Frequency: " << sig.frequency << " Hz\n";
    file << "# Amplitude: " << sig.amplitude << " V\n";
    file << "# Phase: " << sig.phase << " rad\n";
    file << "# Offset: " << sig.offset << " V\n";
    file << "Time (s),Voltage\n";
    
    for (size_t i = 0; i < sig.data.size(); ++i) {
        file << fixed << setprecision(6) << (i / (double)sig.data.size()) 
             << "," << sig.data[i] << "\n";
    }
    
    file.close();
    cout << "Signal saved to " << filename << " successfully.\n";    
};


//load
void load_signal(vector<Signal>& signals) {
    string filename;
    cout << "Enter filename to load: ";
    cin >> filename;
    
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Could not open file for reading.\n";
        return;
    }
    
    Signal sig;
    string line;
    
    while (getline(file, line)) {
        if (line[0] == '#') {
            if (line.find("Signal Type:") != string::npos) {
                sig.type = line.substr(line.find(":") + 2);
            }
            else if (line.find("Frequency:") != string::npos) {
                sig.frequency = stod(line.substr(line.find(":") + 2));
            }
            else if (line.find("Amplitude:") != string::npos) {
                sig.amplitude = stod(line.substr(line.find(":") + 2));
            }
            else if (line.find("Phase:") != string::npos) {
                sig.phase = stod(line.substr(line.find(":") + 2));
            }
            else if (line.find("Offset:") != string::npos) {
                sig.offset = stod(line.substr(line.find(":") + 2));
            }
        }
        else if (line.find("Time,Voltage") != string::npos) {
            break;
        }
    }
    
    double time, voltage;
    char comma;
    while (file >> time >> comma >> voltage) {
        sig.data.push_back(voltage);
    }
    
    file.close();
    signals.push_back(sig);
    cout << "Signal loaded from " << filename << " successfully.\n";
}


// RC circuit


Signal simulate_rc_circuit(const Signal& input_sig, double R, double C, double fs) {
    int N = (int)input_sig.data.size();
    double T = 1.0 / fs;
    double RC = R * C;
    double alpha = RC / (RC + T);


    vector<double> output(N);
    output[0] = 0.0;


    for (int n = 1; n < N; ++n) {
        double x = input_sig.data[n];
        output[n] = alpha * output[n - 1] + (1.0 - alpha) * x;
    }


    Signal out_sig;
    out_sig.data = output;
    out_sig.type = "RC_output";
    out_sig.frequency = input_sig.frequency;
    out_sig.amplitude = input_sig.amplitude;
    out_sig.phase = input_sig.phase;
    out_sig.offset = input_sig.offset;
    return out_sig;
}


// property comps


double compute_peak_to_peak(const Signal& sig, int start_idx, int end_idx) {
    if (sig.data.empty()) return 0.0;
    if (start_idx < 0) start_idx = 0;
    if (end_idx >= (int)sig.data.size()) end_idx = (int)sig.data.size() - 1;
    if (start_idx > end_idx) return 0.0;


    double vmin = sig.data[start_idx];
    double vmax = sig.data[start_idx];
    for (int i = start_idx; i <= end_idx; ++i) {
        if (sig.data[i] < vmin) vmin = sig.data[i];
        if (sig.data[i] > vmax) vmax = sig.data[i];
    }
    return vmax - vmin;
}


// total energy
double compute_total_energy(const Signal& sig, double dt, int start_idx, int end_idx) {
    if (sig.data.empty()) return 0.0;
    if (start_idx < 0) start_idx = 0;
    if (end_idx >= (int)sig.data.size()) end_idx = (int)sig.data.size() - 1;
    if (start_idx > end_idx) return 0.0;


    double sum = 0.0;
    for (int i = start_idx; i <= end_idx; ++i) {
        sum += sig.data[i] * sig.data[i] * dt;
    }
    return sum;
}


// RMS over a range
double compute_rms(const Signal& sig, int start_idx, int end_idx) {
    if (sig.data.empty()) return 0.0;
    if (start_idx < 0) start_idx = 0;
    if (end_idx >= (int)sig.data.size()) end_idx = (int)sig.data.size() - 1;
    if (start_idx > end_idx) return 0.0;


    int N = end_idx - start_idx + 1;
    double sum_sq = 0.0;
    for (int i = start_idx; i <= end_idx; ++i) {
        sum_sq += sig.data[i] * sig.data[i];
    }
    return sqrt(sum_sq / (double)N);
}


// trapezoidal num integration
double compute_trapezoidal_area(const Signal& sig, double dt, int start_idx, int end_idx) {
    if (sig.data.empty()) return 0.0;
    if (start_idx < 0) start_idx = 0;
    if (end_idx >= (int)sig.data.size()) end_idx = (int)sig.data.size() - 1;
    if (start_idx >= end_idx) return 0.0;


    double area = 0.0;
    area += 0.5 * sig.data[start_idx];
    for (int i = start_idx + 1; i < end_idx; ++i) {
        area += sig.data[i];
    }
    area += 0.5 * sig.data[end_idx];


    return area * dt;
}


// save RC

void save_signal_rc(const Signal& sig, const string& filename, double fs) {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Could not open file for writing.\n";
        return;
    }


    file << "# Type: " << sig.type << "\n";
    file << "# Samples: " << sig.data.size() << "\n";
    file << "# fs: " << fs << " Hz\n";
    file << "Time (s),Voltage\n";


    for (size_t i = 0; i < sig.data.size(); ++i) {
        double t = i / fs;
        file << fixed << setprecision(6) << t << "," << sig.data[i] << "\n";
    }


    file.close();
    cout << "Signal saved to " << filename << endl;
}

struct FourDPoint {
    double time;      
    double current;
    double voltage;   
    double resistance; 
};

vector<FourDPoint> create_4d_array(int N,
                                   double I_dc,
                                   double R_min,
                                   double R_max,
                                   double V_noise_std,
                                   double fs) {
    vector<FourDPoint> data(N);
    double dt = 1.0 / fs;

    for (int n = 0; n < N; ++n) {
        double t = n * dt;

        double alpha = (N > 1) ? (double)n / (double)(N - 1) : 0.0;
        double R = R_min + alpha * (R_max - R_min);

        double noise = V_noise_std * ( (rand() / (double)RAND_MAX) - 0.5 ) * 2.0;

        double I = I_dc;
        double V = I * R + noise;

        data[n].time = t;
        data[n].current = I;
        data[n].voltage = V;
        data[n].resistance = R;
    }

    return data;
}

double compute_average_power(const vector<FourDPoint>& data) {
    if (data.empty()) return 0.0;
    double sum = 0.0;
    for (size_t i = 0; i < data.size(); ++i) {
        sum += data[i].voltage * data[i].current;
    }
    return sum / (double)data.size(); 
}

double compute_average_resistance(const vector<FourDPoint>& data) {
    if (data.empty()) return 0.0;
    double sum = 0.0;
    for (size_t i = 0; i < data.size(); ++i) {
        sum += data[i].resistance;
    }
    return sum / (double)data.size(); 
}

double compute_total_energy_4d(const vector<FourDPoint>& data, double dt) {
    if (data.empty()) return 0.0;
    double E = 0.0;
    for (size_t i = 0; i < data.size(); ++i) {
        double P = data[i].voltage * data[i].current; 
        E += P * dt; 
    }
    return E;
}

void save_4d_array_to_csv(const vector<FourDPoint>& data,
                          const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Could not open file for writing.\n";
        return;
    }

    file << "# 4D array: time,current,voltage,resistance\n";
    file << "time (s), current (A), voltage (V), resistance (ohms)\n";

    file << fixed << setprecision(6);
    for (size_t i = 0; i < data.size(); ++i) {
        file << data[i].time << ","
             << data[i].current << ","
             << data[i].voltage << ","
             << data[i].resistance << "\n";
    }

    file.close();
    cout << "4D array saved to " << filename << "\n";
}


int main() {
    vector<Signal> signals;
    vector<string> signal_types;
    Signal rc_output;
    bool has_rc_output = false;


    double default_fs = 1000.0; 
    double offset = 0.0; 
    double frequency = 1000; 
    double amplitude = 1;
    double phase = 0; 
    vector<FourDPoint> fourD_data;

    int choice;
    do {

        menu();
        cout << "Enter your choice (1-12): ";
        
        while (!(cin >> choice)|| choice < 1 || choice > 12) {
            cout << "Invalid choice. Please enter a valid menu option (1-10): ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }


        if (choice >= 1 and choice <= 3) {
            string type;
            double frequency = get_double("Set frequency: (Hz)");
            double amplitude = get_double("Set amplitude: (V)");
            double phase = get_double("Set Phase (Radians): ");
            double offset = get_double("Set offset: (V)");

            if (choice == 1) type = "sine";
            if (choice == 2) type = "square";
            if (choice == 3) type = "triangle";
            
            Signal sig;
            sig.type = type;
            sig.frequency = frequency;
            sig.amplitude = amplitude;
            sig.phase = phase;
            sig.offset = offset;
            sig.data = generate_signal(type, frequency, amplitude, phase, offset);


            signals.push_back(sig);
            signal_types.push_back(sig.type);


            cout << type << " signal generated and stored.\n";
        } 
        
        else if (choice == 4) {
            display_signal(signals);
        }
        
        else if (choice == 5) {
            if (!has_rc_output) {
                cout << "No RC output signal yet. Run RC simulation first.\n";
            } else {
                signal_properties_menu();
                cout << "Enter property choice (1-4): ";
                int pchoice;
                cin >> pchoice;


                int start_idx = 0;
                int end_idx = (int)rc_output.data.size() - 1;
                cout << "Enter start index (0-" << end_idx << "): ";
                cin >> start_idx;
                cout << "Enter end index (0-" << end_idx << "): ";
                cin >> end_idx;


                double dt = 1.0 / default_fs;
                double result = 0.0;
                string prop_name;
                string unit;


                if (pchoice == 1) {
                    prop_name = "Peak-to-peak amplitude";
                    result = compute_peak_to_peak(rc_output, start_idx, end_idx);
                    unit = "V";
                } else if (pchoice == 2) {
                    prop_name = "Total energy";
                    result = compute_total_energy(rc_output, dt, start_idx, end_idx);
                    unit = "V^2 * s";
                } else if (pchoice == 3) {
                    prop_name = "RMS value";
                    result = compute_rms(rc_output, start_idx, end_idx);
                    unit = "V";
                } else if (pchoice == 4) {
                    prop_name = "Trapezoidal area (integration)";
                    result = compute_trapezoidal_area(rc_output, dt, start_idx, end_idx);
                    unit = "V * s";
                } else {
                    cout << "Invalid property choice.\n";
                }


                if (!prop_name.empty()) {
                    cout << prop_name << " = " << result << unit << endl;


                    ofstream f("rc_properties.txt", ios::app);
                    if (f.is_open()) {
                        f << prop_name << " (start=" << start_idx
                          << ", end=" << end_idx << ") = " << result << "\n";
                        f.close();
                    }
                }
            }
        }
        
        else if (choice == 6) {
            load_signal(signals);
        }
        
        else if (choice == 7) {
            save_signal(signals);
        }

        else if (choice == 8) {
            if (signals.empty()) {
                cout << "No raw signal available. Generate one first.\n";
            } else {
                double R = get_double("Enter R (ohms) for RC circuit: ");
                double C = get_double("Enter C (farads) for RC circuit: ");
                double fs = default_fs;
                rc_output = simulate_rc_circuit(signals.back(), R, C, fs);
                has_rc_output = true;
                cout << "RC circuit simulation complete. Output stored.\n";
            }
        }

        else if (choice == 9) {
            if (!has_rc_output) {
                cout << "No RC output to save.\n";
            } else {
                string filename;
                cout << "Enter filename for RC output (e.g., rc_output.csv): ";
                cin >> filename;
                save_signal_rc(rc_output, filename, default_fs);
            }
        }
        else if (choice == 10) {
            int N;
            cout << "Enter number of timepoints (samples): ";
            cin >> N;
            if (N <= 0) {
                cout << "Number of samples must be positive.\n";
                continue;
            }

            double fs = default_fs; 
            double I_dc = get_double("Enter DC current (A): ");
            double R_min = get_double("Enter minimum resistance (ohms): ");
            double R_max = get_double("Enter maximum resistance (ohms): ");
            double V_noise_std = get_double("Enter voltage noise std dev (V): ");

            fourD_data = create_4d_array(N, I_dc, R_min, R_max, V_noise_std, fs);

            double dt = 1.0 / fs;
            double P_avg = compute_average_power(fourD_data);
            double R_avg = compute_average_resistance(fourD_data);
            double E_total = compute_total_energy_4d(fourD_data, dt);

            cout << "\n4D array initialized with realistic values.\n";
            cout << "Average power = " << P_avg << " W\n";
            cout << "Average resistance = " << R_avg << " ohms\n";
            cout << "Total energy dissipated = " << E_total << " J\n";
            cout << "4D Array stored. ";
    }
    else if (choice == 11){
        string filename;
        cout << "Enter filename to save 4D array (e.g., 4dArrays.csv): ";
        cin >> filename;
        save_4d_array_to_csv(fourD_data, filename);
    }

    } 
    while (choice != 12);
    cout << "Exiting program. \n";
    return 0;
}
