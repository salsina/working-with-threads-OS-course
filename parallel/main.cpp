#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <chrono> 
#include <pthread.h>
#include <sstream>
#define NUMBER_OF_THREADS 4
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

struct thread_data
{
    string train_adr;
    vector<float> mins;
    vector<float> maxes;
    vector <device*> devices;
    vector <price_class*> price_classes;
    vector<float> final_mins;
    vector<float> final_maxes;
    float matches ;
};

vector <price_class*> read_weights(string adr)
{
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
    return price_classes;
}

void* read_devices(void* data)
{
    struct thread_data* my_data = (struct thread_data*) data;
    string adr = my_data->train_adr;
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
    my_data->mins = mins;
    my_data->maxes = maxes;
    my_data->devices = devices;
	pthread_exit((void*) my_data);

}


void normalize_devices(vector <device*> devices,vector<float> mins,vector<float> maxes)
{
    for(int i = 0; i<devices.size();i++)
    {
        devices[i]->normalize_weights(mins,maxes);
    }
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
}

void print_accurancy(float num_of_devices, float matches)
{
    float answer = (matches / num_of_devices) * 100;
    cout<<"Accuracy: "<<fixed<<setprecision(2)<<answer<<"%"<<endl;
}

void* normalize_and_calculate(void* data)
{
    struct thread_data* my_data = (struct thread_data*) data;
    vector<device*> devices = my_data->devices;
    vector <price_class*> price_classes = my_data->price_classes;
    vector<float> mins = my_data->final_mins;
    vector<float> maxes = my_data->final_maxes;
    normalize_devices(devices,mins,maxes);
    calculate_estimated_price_ranges(devices, price_classes);
    float matches = 0;
    for(int i=0;i<devices.size();i++)
    {
        if (devices[i]->ranges_match())
            matches++ ;
    }
    my_data->matches = matches;
    pthread_exit((void*)my_data);

}
int main(int argc,char* argv[]){
    if(argc > 2)
    {
        cout<<"Wrong Input\n";
        exit(1);
    }
    struct thread_data thread_data_array[NUMBER_OF_THREADS];
    string datasets_adr = argv[1];
    vector <price_class*> price_classes;
    vector <vector <device*>> devices;
    string weights_adr = datasets_adr + "/" + "weights.csv";
    price_classes = read_weights(weights_adr);
    pthread_t threads[NUMBER_OF_THREADS];
    void* status;

    for(int i=0;i<NUMBER_OF_THREADS;i++)
    {
        string train_adr = datasets_adr + "/" + "train_" + to_string(i) + ".csv";
        thread_data_array[i].train_adr = train_adr;
        thread_data_array[i].price_classes = price_classes;

        int return_code = pthread_create(&threads[i], NULL, read_devices,
				(void*)&thread_data_array[i]);
		if (return_code)
		{
			cout<<("ERROR; return code from pthread_create() is " + return_code)<<endl;
			exit(-1);
		}
    }
    
    vector<vector<float>>mins;
    vector<vector<float>>maxes;

    for(long tid = 0; tid < NUMBER_OF_THREADS; tid++)
	{
		int return_code = pthread_join(threads[tid], &status);
		if (return_code)
		{
			cout<<("ERROR; return code from pthread_join() is " + return_code)<<endl;
			exit(-1);
		}
        struct thread_data* my_data = (struct thread_data*) status;

        mins.push_back(my_data->mins);
        maxes.push_back(my_data->maxes);
        devices.push_back(my_data->devices);
	}
    vector<float> final_mins = mins[0];
    vector<float> final_maxes = maxes[0];
    for(int i=1;i<maxes.size();i++)
    {
        for(int j=0;j<maxes[i].size();j++)
        {
            if (final_maxes[j] < maxes[i][j])
                final_maxes[j] = maxes[i][j];
        }
    }
    
    for(int i=1;i<mins.size();i++)
    {
        for(int j=0;j<mins[i].size();j++)
        {
            if (final_mins[j] > mins[i][j])
                final_mins[j] = mins[i][j];
        }
    }

    for(int i = 0;i<devices.size();i++)
    {
        thread_data_array[i].final_mins = final_mins;
        thread_data_array[i].final_maxes = final_maxes;
        int return_code = pthread_create(&threads[i], NULL, normalize_and_calculate,
        (void*)&thread_data_array[i]);
		if (return_code)
		{
			cout<<("ERROR; return code from pthread_create() is " + return_code)<<endl;
			exit(-1);
		}
    }

    float final_matches = 0;
    float num_of_devices = 0;
    for(long tid = 0; tid < NUMBER_OF_THREADS; tid++)
	{
		int return_code = pthread_join(threads[tid], &status);
		if (return_code)
		{
			cout<<("ERROR; return code from pthread_join() is " + return_code)<<endl;
			exit(-1);
		}
        struct thread_data* my_data = (struct thread_data*) status;
        final_matches += my_data->matches;
        num_of_devices += my_data->devices.size();
	}

    print_accurancy(num_of_devices, final_matches);
    pthread_exit(NULL);
    return 0;
}
