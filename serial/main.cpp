#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <chrono> 
using namespace std;
using namespace chrono; 

class price_class{
public:
    void set_weights(vector<float> w)
    {
        weights = w;
        bias = w[w.size()-1];
    }

    vector<float> get_weights() {return weights;}

    float get_bias() {return bias;}
private:
    vector<float> weights;
    float bias;
};

class device{
public:
    void set_weights(vector<float> w)
    {
        weights = w;
        price_range = w[w.size() - 1] ;
    }

    vector<float> get_weights() {return weights;}

    void normalize_weights(vector<float> mins,vector<float> maxes)
    {
        for(int i=0; i<weights.size() - 1;i++)
        {
            weights[i] = (weights[i] - mins[i]) / (maxes[i] - mins[i]) ;
        }
    }

    void set_estimated_price_range(int num) { estimated_price_range = num; }

    bool ranges_match()
    {
        if(price_range == estimated_price_range)
            return true;
        return false;
    }

private:
    vector<float> weights;
    int price_range;
    int estimated_price_range;
};

vector <price_class*> read_weights(string adr)
{
    // auto start = high_resolution_clock::now();
    string line;
    ifstream file;
    file.open(adr);
    getline(file,line,'\n');
    vector <price_class*> price_classes;
    while(getline(file,line,'\n'))
    {
        price_class* p = new price_class;
        vector <float> weights;
        stringstream check1(line); 
        string intermediate; 
        while(getline(check1, intermediate, ',')) 
        { 
            weights.push_back(stof(intermediate)); 
        } 
        p->set_weights(weights);
        price_classes.push_back(p);     
    }
    file.close();
    // auto stop = high_resolution_clock::now(); 
    // auto duration = duration_cast<microseconds>(stop - start); 
    // cout << duration.count() << endl; 

    return price_classes;
}

vector <device*> read_devices(string adr)
{
    // auto start = high_resolution_clock::now();
    string line;
    ifstream file;
    file.open(adr);
    getline(file,line,'\n');
    vector <device*> devices;
    
    while(getline(file,line,'\n'))
    {
        device* d = new device;
        vector <float> weights;
        stringstream check1(line); 
        string intermediate; 

        while(getline(check1, intermediate, ',')) 
        { 
            weights.push_back(stof(intermediate)); 
        } 
        d->set_weights(weights);
        devices.push_back(d); 
    }
    file.close();
    // auto stop = high_resolution_clock::now(); 
    // auto duration = duration_cast<microseconds>(stop - start); 
    // cout << duration.count() << endl; 
    return devices;
}

void normalize_devices(vector <device*> devices)
{
    // auto start = high_resolution_clock::now();
    int num_of_elements = devices[0]->get_weights().size();
    vector <float> mins;
    vector <float> maxes;

    for(int i =0; i<num_of_elements - 1;i++)
    {
        float min = devices[0]->get_weights()[i];
        float max = min;
        for(int j =0;j<devices.size();j++)
        {
            if(devices[j]->get_weights()[i] > max) 
                max = devices[j]->get_weights()[i];
            if(devices[j]->get_weights()[i] < min) 
                min = devices[j]->get_weights()[i];
        }
        mins.push_back(min);
        maxes.push_back(max);
    }

    for(int i = 0; i<devices.size();i++)
    {
        devices[i]->normalize_weights(mins,maxes);
    }
    // auto stop = high_resolution_clock::now(); 
    // auto duration = duration_cast<microseconds>(stop - start); 
    // cout << duration.count() << endl; 

}

int calculate_max_answer_index(vector<float> answers)
{
    float max = answers[0];
    int index = 0;
    for(int i=0;i<answers.size();i++)
    {
        if(answers[i] > max)
        {
            max = answers[i];
            index = i;
        }
    }
    return index;
}

void calculate_estimated_price_ranges(vector <device*> devices, vector <price_class*> price_classes)
{
    // auto start = high_resolution_clock::now();
    int num_of_elements = devices[0]->get_weights().size();

    for (int i =0;i<devices.size();i++)
    {
        device* cur_device = devices[i];
        vector<float> answers;

        for(int j=0;j<price_classes.size();j++)
        {
            price_class* cur_price_class = price_classes[j];
            float sum = 0;
            for(int k=0;k<num_of_elements-1;k++)
            {
                sum += (cur_device->get_weights()[k] * cur_price_class->get_weights()[k]);
            }
            sum += cur_price_class->get_bias();
            answers.push_back(sum);
        }

        int index = calculate_max_answer_index(answers);
        devices[i]->set_estimated_price_range(index);   
    }
    // auto stop = high_resolution_clock::now(); 
    // auto duration = duration_cast<microseconds>(stop - start); 
    // cout << duration.count() << endl; 

}

void print_accurancy(vector <device*> devices)
{
    // auto start = high_resolution_clock::now();
    float num_of_devices = devices.size();
    float matches = 0;
    for(int i=0;i<num_of_devices;i++)
    {
        if (devices[i]->ranges_match())
            matches++ ;
    }

    float answer = (matches / num_of_devices) * 100;
    cout<<"Accuracy: "<<fixed<<setprecision(2)<<answer<<"%"<<endl;
    // auto stop = high_resolution_clock::now(); 
    // auto duration = duration_cast<microseconds>(stop - start); 
    // cout << duration.count() << endl; 

}

int main(int argc,char* argv[]){
    // auto start = high_resolution_clock::now();
    if(argc > 2)
    {
        cout<<"Wrong Input\n";
        exit(1);
    }
    string datasets_adr = argv[1];
    vector <price_class*> price_classes;
    vector <device*> devices;
    string weights_adr = datasets_adr + "/" + "weights.csv";
    string train_adr = datasets_adr + "/" + "train.csv";
    price_classes = read_weights(weights_adr);//180
    devices = read_devices(train_adr);//14000
    normalize_devices(devices);//15000
    calculate_estimated_price_ranges(devices, price_classes);//48000
    print_accurancy(devices);//120
    // auto stop = high_resolution_clock::now(); 
    // auto duration = duration_cast<microseconds>(stop - start); 
    // cout << duration.count() << endl; 
    return 0;
}
