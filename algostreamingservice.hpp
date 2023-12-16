/**
* algostreamingservice.hpp
* Defines the data types and Service for algo executions.
*
* @author Breman Thuraisingham & Tengxiao Fan
*/

#ifndef  ALGOSTREAMINGSERVICE_HPP
#define ALGOSTREAMINGSERVICE_HPP

#include <string>
#include "soa.hpp"
#include "pricingservice.hpp"
#include "marketdataservice.hpp"

/*
* 
*/
template <typename T>
class PriceStream
{
private:
	T product;
	double bidprice;
	double offerprice;
	long visiblequantity;
	long hiddenquantity;
public:
	//Ctor and Dtor
	PriceStream() {}
	PriceStream(T pdt,double bidp, double offerp, long visible_quantity, long hidden_quantity)
	{
		product = pdt;
		bidprice = bidp;
		offerprice = offerp;
		visiblequantity = visible_quantity;
		hiddenquantity = hidden_quantity;
	}
	~PriceStream() = default;

	//Getter functions
	const T& GetProduct() const
	{
		return product;
	}
	double GetBidPrice() const
	{
		return bidprice;
	}
	double GetOfferPrice() const
	{
		return offerprice;
	}
	long GetVisibleQuantity() const
	{
		return visiblequantity;
	}
	long GetHiddenQuantity() const
	{
		return hiddenquantity;
	}
	//Output function
	ostream& Output(ostream& file)
	{
		file << GetProduct().GetProductId() << "," << GetBidPrice() << "," << GetOfferPrice() << "," << GetVisibleQuantity() << "," << GetHiddenQuantity() << endl;
		return file;
	}
};


/*
*/
template<typename T>
class AlgoStream
{
private:
	PriceStream<T>* pricestream;
public:
	//Ctor and Dtor
	AlgoStream() {}
	AlgoStream(T pdt, double bidp, double offerp, long visible_quantity, long hidden_quantity)
	{
		pricestream = new PriceStream<T>(pdt, bidp, offerp, visible_quantity, hidden_quantity);
	}
	~AlgoStream() = default;
	//Get the order
	PriceStream<T>* GetPriceStream()
	{
		return pricestream;
	}
};

//pre declaration, Listener to pricing
template <typename T>
class AlgoStreamingPricingListener;

template <typename T>
class AlgoStreamingService : public Service<string, AlgoStream<T>>
{
private:
	map<string, AlgoStream<T>> algostreammap;
	vector<ServiceListener<AlgoStream<T>>*> listeners;
	AlgoStreamingPricingListener<T>* PricingListener;
	bool switcher;//decide the quantity
public:
	//Ctor and Dtor
	AlgoStreamingService()
	{
		algostreammap= map<string, AlgoStream<T>>();
		listeners= vector<ServiceListener<AlgoStream<T>>*>();
		PricingListener = new AlgoStreamingPricingListener<T>(this);
		switcher = 0;
	}
	~AlgoStreamingService() = default;

	// Get data on our service given a key
	AlgoStream<T>& GetData(string key)
	{
		return algostreammap[key];
	}

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(AlgoStream<T>& data)
	{
		string key = data.GetPriceStream()->GetProduct().GetProductId();
		algostreammap[key] = data;
		for (auto i = listeners.begin(); i != listeners.end(); i++)
		{
			(*i)->ProcessAdd(data);
		}
	}

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<AlgoStream<T>>* listener)
	{
		listeners.push_back(listener);
	}

	// Get all listeners on the Service
	const vector<ServiceListener<AlgoStream<T>>*>& GetListeners() const
	{
		return listeners;
	}

	// Get the listener of the service
	AlgoStreamingPricingListener<T>* GetPricingListener()
	{
		return PricingListener;
	}

	void PublishPrice(Price<T>& price)
	{
		T product = price.GetProduct();
		string productid = product.GetProductId();
		double mid = price.GetMid();
		double spread = price.GetBidOfferSpread();
		double bid = mid - spread / 2.0;
		double offer = mid + spread / 2.0;
		long visiblequantity = (switcher + 1) * 1000000;
		switcher = 1 - switcher;
		long hiddenquantity = 2 * visiblequantity;

		AlgoStream<T> algostream(product, bid, offer, visiblequantity, hiddenquantity);
		OnMessage(algostream);
		//std::cout << productid << "," << bid << "," << offer << "," << visiblequantity << "," << hiddenquantity<<endl;
	}

};


/*
* Algostream listener to pricing service
*/
template<typename T>
class AlgoStreamingPricingListener :public ServiceListener<Price<T>>
{
private:
	AlgoStreamingService<T>* service;
public:
	AlgoStreamingPricingListener(AlgoStreamingService<T>* s)
	{
		service = s;
	}

	// Listener callback to process an add event to the Service
	void ProcessAdd(Price<T>& data)
	{
		service->PublishPrice(data);
	}

	// Listener callback to process a remove event to the Service
	void ProcessRemove(Price<T>& data){}

	// Listener callback to process an update event to the Service
	void ProcessUpdate(Price<T>& data){}
};

#endif // ! ALGOSTREAMINGSERVICE_HPP
