/**
 * pricingservice.hpp
 * Defines the data types and Service for internal prices.
 *
 * @author Breman Thuraisingham & Tengxiao Fan
 */
#ifndef PRICING_SERVICE_HPP
#define PRICING_SERVICE_HPP

#include <string>
#include "soa.hpp"

/**
 * A price object consisting of mid and bid/offer spread.
 * Type T is the product type.
 */
template<typename T>
class Price
{

public:

  // ctor for a price
	Price()=default;
  Price(const T &_product, double _mid, double _bidOfferSpread);

  // Get the product
  const T& GetProduct() const;

  // Get the mid price
  double GetMid() const;

  // Get the bid/offer spread around the mid
  double GetBidOfferSpread() const;

private:
  T product;
  double mid;
  double bidOfferSpread;

};

template<typename T>
Price<T>::Price(const T &_product, double _mid, double _bidOfferSpread) :
  product(_product)
{
  mid = _mid;
  bidOfferSpread = _bidOfferSpread;
}

template<typename T>
const T& Price<T>::GetProduct() const
{
  return product;
}

template<typename T>
double Price<T>::GetMid() const
{
  return mid;
}

template<typename T>
double Price<T>::GetBidOfferSpread() const
{
  return bidOfferSpread;
}

/*
* Pre-definition of a pricing connector
*/
template<typename T>
class PricingConnector;

/**
 * Pricing Service managing mid prices and bid/offers.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PricingService : public Service<string, Price <T> >
{
private:
	//prices of each lines
	map<string, Price<T>> prices;
	vector <ServiceListener<Price<T>>*> listeners;
	PricingConnector<T>* connector;
	
public:
	//Ctor and Dtor (default)
	PricingService()
	{
		prices = map<string, Price<T>>();
		listeners = vector<ServiceListener<Price<T>>*>();
		connector = new PricingConnector<T>(this);
	}
	~PricingService() = default;

	//Get the price of a key
	virtual Price<T>& GetData(string key)
	{
		return prices[key];
	}

	// The callback that a Connector should invoke for any new or updated data
	virtual void OnMessage(Price<T>& data)
	{
		//renew the price in the map
		string key = data.GetProduct().GetProductId();
		prices[key] = data;

		//Add the process to all the listeners
		for (auto i = listeners.begin(); i != listeners.end(); i++)
		{
			(*i)->ProcessAdd(data);
		}
	}

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	virtual void AddListener(ServiceListener<Price<T>>* l)
	{
		listeners.push_back(l);
	}

	//Get all the listeners
	const vector<ServiceListener<Price<T>>*>& GetListeners() const
	{
		return listeners;
	}

	//Get the connector
	PricingConnector<T>* GetConnector()
	{
		return connector;
	}

};


/*
* The definition of pricing connector class
*/

template<typename T>
class PricingConnector : public Connector<Price<T>>
{
private:
	PricingService<T>* service;
	string file_name;

public:
	//Ctor and Dtor
	PricingConnector(PricingService<T>* s)
	{
		//file_name = f;
		service = s;
	}
	~PricingConnector()=default;

	//Publish data (Nothing to realize)
	void Publish(Price<T>& data) {}

	//Subscribe data
	void Subscribe(ifstream& data)
	{
		string line;
		
		while (getline(data, line))
		{
			string ele = "";
			vector<string> elements;
			for (int i = 0; i < line.size(); i++)
			{
				if (line[i] == ',')
				{
					elements.push_back(ele);
					ele = "";
				}
				else
				{
					ele.push_back(line[i]);
				}
			}
			elements.push_back(ele);
			//std::cout << elements[2] << endl;
			string cusip = elements[0];
			double bid = FractionaltoPrice(elements[1]);
			double offer = FractionaltoPrice(elements[2]);
			double mid = (bid + offer) / 2;
			double spread = offer - bid;
			T product = MakeBond(cusip);
			Price<T> p(product, mid, spread);
			service->OnMessage(p);
			//std::cout << cusip << "," << mid << "," << spread<<std::endl;
		}
	}
};


#endif
