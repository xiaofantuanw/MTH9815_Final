/**
 * historicaldataservice.hpp
 * historicaldataservice.hpp
 *
 * @author Breman Thuraisingham & Tengxiao
 * Defines the data types and Service for historical data.
 *
 * @author Breman Thuraisingham & Tengxiao
 */
#ifndef HISTORICAL_DATA_SERVICE_HPP
#define HISTORICAL_DATA_SERVICE_HPP
#include "soa.hpp"
#include "positionservice.hpp"
#include "tradebookingservice.hpp"
#include "inquiryservice.hpp"
#include <iomanip>

template <typename T>
class HistoricalDataConnector;
template<typename T>
class HistoricalDataListener;



/**
 * Service for processing and persisting historical data to a persistent store.
 * Keyed on some persistent key.
 * Type T is the data type to persist.
 */
template<typename T>
class HistoricalDataService : Service<string,T>
{
private:
	map<string, T> historicaldatamap;
	vector<ServiceListener<T>*> listeners;
	HistoricalDataConnector<T>* connector;
	HistoricalDataListener<T>* DataListener;
	string type;

public:
	//Ctor and Dtor
	HistoricalDataService()
	{
		historicaldatamap = map<string, T>();
		listeners = vector<ServiceListener<T>*>();
		connector = new HistoricalDataConnector<T>(this);
		DataListener = new HistoricalDataListener<T>(this);
		type = "POSITION";
	}

	HistoricalDataService(string t)
	{
		historicaldatamap = map<string, T>();
		listeners = vector<ServiceListener<T>*>();
		connector = new HistoricalDataConnector<T>(this);
		DataListener = new HistoricalDataListener<T>(this);
		type = t;
	}

	~HistoricalDataService() = default;


	// Get data on our service given a key
	virtual T& GetData(string key)
	{
		return historicaldatamap[key];
	}

	// The callback that a Connector should invoke for any new or updated data
	virtual void OnMessage(T& data)
	{
		string key = data.GetProduct().GetProductId();
		historicaldatamap[key] = data;
		//Notify all the listeners
		for (auto i = listeners.begin(); i != listeners.end(); i++)
		{
			(*i)->ProcessAdd(data);
		}
		connector->Publish(data);
	}

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	virtual void AddListener(ServiceListener<T>* listener)
	{
		listeners.push_back(listener);
	}

	// Get all listeners on the Service
	virtual const vector<ServiceListener<T>*>& GetListeners() const
	{
		return listeners;
	}

	// Get the connector of the service
	virtual HistoricalDataConnector<T>* GetConnector()
	{
		return connector;
	}

	// Get the listener of the service
	virtual HistoricalDataListener<T>* GetDataListener()
	{
		return DataListener;
	}

	// Get the service type that historical data comes from
	string GetType() const
	{
		return type;
	}
  // Persist data to a store
	void PersistData(string persistKey, T& data)
	{
		connector->Publish(data);
	}
};

/*
* HistoricalData Connector
*/
template <typename T>
class HistoricalDataConnector :public Connector<T>
{
private:
	HistoricalDataService<T>* service;

public:
	//Ctor and Dtor
	HistoricalDataConnector(HistoricalDataService<T>* s)
	{
		service = s;
	}
	~HistoricalDataConnector() = default;

	//Publisher
	void Publish(T& data)
	{

		string type = service->GetType();
		ofstream file;
		file << std::fixed << std::setprecision(6) << std::endl;
		if (type == "POSITION")
		{
			file.open("positions.txt", ios::app);
			
			//string trsy1 = "TRSY1";
			//string trsy2 = "TRSY2";
			//string trsy3 = "TRSY3";
			//file << data.GetProduct().GetProductId() << ",TRSY1," << data.GetPosition(trsy1) << ",TRSY2," << data.GetPosition(trsy2) << ",TRSY3," << data.GetPosition(trsy3) << ",TOTAL," << data.GetAggregatePosition() << endl;
		}
		else if (type == "RISK")
		{
			file.open("risk.txt", ios::app);
			//file << data.GetProduct().GetProductId() << "," << data.GetPV01() << "," << data.GetQuantity() << endl;
		}
		else if (type == "EXECUTION")
		{
			file.open("execution.txt", ios::app);
		}
		else if (type == "STREAMING")
		{
			file.open("streaming.txt", ios::app);
			//cout << "Writing Start" << endl;
			//file << data.GetProduct().GetProductId() << "," << data.GetBidPrice() << "," << data.GetOfferPrice() << "," << data.GetVisibleQuantity() << "," << data.GetHiddenQuantity()<<endl;
		}
		else if (type == "INQUIRY")
		{
			file.open("allinquiries.txt", ios::app);
		}
		data.Output(file);
		//file.close();
		//if (type == "STREAMING") cout << "Writing end" << endl;
	}
	//Subscriber
	void Subscribe(ifstream& data) {}
	
};


template <typename T>
class HistoricalDataListener : public ServiceListener<T>
{
private:
	HistoricalDataService<T>* service;
	
public:
	//Ctor and Dtor
	HistoricalDataListener(HistoricalDataService<T>* s)
	{
		service = s;
	}
	~HistoricalDataListener() = default;

	// Listener callback to process an add event to the Service
	void ProcessAdd(T& data)
	{
		string key = data.GetProduct().GetProductId();
		service->PersistData(key, data);
	}

	// Listener callback to process a remove event to the Service
	void ProcessRemove(T& data){}

	// Listener callback to process an update event to the Service
	void ProcessUpdate(T& data){}
};

#endif
