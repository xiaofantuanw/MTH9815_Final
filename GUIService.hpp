/**
 * executionservice.hpp
 * Defines the data types and Service for GUIs.
 *
 * @author Breman Thuraisingham & Tengxiao Fan
 */
#ifndef GUISERVICE_HPP
#define GUISERVICE_HPP

#include "soa.hpp"
#include "pricingservice.hpp"
#include <iomanip>

/*
* Pre declaration for connector and listener
*/
template<typename T>
class GUIConnector;
template<typename T>
class GUIPricingListener;

/*
* GUI service
*/

template<typename T>
class GUIService : Service<string, Price<T>>
{
private:
	map<string, Price<T>> guimap;
	vector<ServiceListener<Price<T>>*> listeners;
	GUIConnector<T>* connector;
	GUIPricingListener<T>* PricingListener;
	long time;

public:
	//Ctor and Dtor
	GUIService() 
	{
		guimap = map<string, Price<T>>();
		listeners= vector<ServiceListener<Price<T>>*>();
		connector = new GUIConnector<T>(this);
		PricingListener = new GUIPricingListener<T>(this);
		time = 0;
	}
	~GUIService() = default;

	// Get data on our service given a key
	Price<T>& GetData(string key)
	{
		return guimap[key];
	}

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(Price<T>& data)
	{
		string key = data.GetProduct().GetProductId();
		guimap[key] = data;
		//Notify all the listeners
		for (auto i = listeners.begin(); i != listeners.end(); i++)
		{
			(*i)->ProcessAdd(data);
		}
		connector->Publish(data);
	}

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<Price<T>>* listener)
	{
		listeners.push_back(listener);
	}

	// Get all listeners on the Service
	const vector<ServiceListener<Price<T>>*>& GetListeners() const
	{
		return listeners;
	}

	// Get the connector of the service
	GUIConnector<T>* GetConnector()
	{
		return connector;
	}

	// Get the listener of the service
	ServiceListener<Price<T>>* GetPricingListener()
	{
		return PricingListener;
	}

	//Get the time
	long GetTime()
	{
		return time;
	}

	//Set the time
	void SetTime(long t)
	{
		time = t;
	}
};


/*
* GUI Connector
*/
template <typename T>
class GUIConnector : public Connector<Price<T>>
{
private:
	GUIService<T>* service;
public:
	//Ctor and Dtor
	GUIConnector(GUIService<T>* s)
	{
		service = s;
	}
	~GUIConnector() = default;
	//Publish Data
	void Publish(Price<T>& data)
	{
		int timenow = service->GetTime();
		//cout << timenow << endl;
		timenow += rand() % 10 + 1;//Add a random time
		service->SetTime(timenow);
		if (timenow-(timenow/300)*300 < 10)
		{
			//service->SetTime(timenow);
			ofstream file;
			file.open("gui.txt", ios::app);
			file << std::fixed << std::setprecision(6);
			file << timenow << "," << data.GetProduct().GetProductId() << "," << data.GetMid() << "," << data.GetBidOfferSpread() << endl;
		}
	}
	//Subscribe data
	void Subscribe(ifstream& data) {}
};

template<typename T>
class GUIPricingListener : public ServiceListener<Price<T>>
{
private:
	GUIService<T>* service;

public:
	//Ctor and Dtor
	GUIPricingListener(GUIService<T>* s)
	{
		service = s;
	}
	~GUIPricingListener() = default;

	// Listener callback to process an add event to the Service
	void ProcessAdd(Price<T>& data)
	{
		service->OnMessage(data);
	}

	// Listener callback to process a remove event to the Service
	void ProcessRemove(Price<T>& data){}

	// Listener callback to process an update event to the Service
	void ProcessUpdate(Price<T>& data){}

};






#endif // !GUISERVICE_HPP
