/**
 * riskservice.hpp
 * Defines the data types and Service for fixed income risk.
 *
 * @author Breman Thuraisingham & Tengxiao Fan
 */
#ifndef RISK_SERVICE_HPP
#define RISK_SERVICE_HPP

#include "soa.hpp"
#include "positionservice.hpp"

/**
 * PV01 risk.
 * Type T is the product type.
 */
template<typename T>
class PV01
{

public:

  // ctor for a PV01 value
	PV01() {}
	PV01(const T &_product, double _pv01, long _quantity);

  // Get the product on this PV01 value
  const T& GetProduct() const
  {
	  return product;
  }

  // Get the PV01 value
  double GetPV01() const
  {
	  return pv01;
  }

  // Get the quantity that this risk value is associated with
  long GetQuantity() const
  {
	  return quantity;
  }

  ostream& Output(ostream& file)
  {
	  file << GetProduct().GetProductId() << "," << GetPV01() << "," << GetQuantity() << endl;
	  return file;
  }

private:
  T product;
  double pv01;
  long quantity;

};

/**
 * A bucket sector to bucket a group of securities.
 * We can then aggregate bucketed risk to this bucket.
 * Type T is the product type.
 */
template<typename T>
class BucketedSector
{

public:

  // ctor for a bucket sector
	BucketedSector() {}
  BucketedSector(const vector<T> &_products, string _name);

  // Get the products associated with this bucket
  const vector<T>& GetProducts() const;

  // Get the name of the bucket
  const string& GetName() const;

private:
  vector<T> products;
  string name;

};


/*
* Pre definition of a listener to the position
*/
template <typename T>
class RiskPositionListener;


/**
 * Risk Service to vend out risk for a particular security and across a risk bucketed sector.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class RiskService : public Service<string,PV01 <T> >
{
private:
	map<string, PV01<T>> pv01map;
	vector<ServiceListener<PV01<T>>*> listeners;
	RiskPositionListener<T>* position_listener;

public:
	//Ctor and Dtor
	RiskService()
	{
		pv01map = map<string, PV01<T>>();
		listeners = vector<ServiceListener<PV01<T>>*>();
		position_listener = new RiskPositionListener<T>(this);
	}

	~RiskService() = default;

	// Get data on our service given a key
	virtual PV01<T>& GetData(string key)
	{
		return pv01map[key];
	}

	// The callback that a Connector should invoke for any new or updated data
	virtual void OnMessage(PV01<T>& data)
	{
		string key = data.GetProduct().GetProductId();
		pv01map[key] = data;
		//Call all the listeners
		for (auto i = listeners.begin(); i != listeners.end(); i++)
		{
			(*i)->ProcessAdd(data);
		}
	}

	// Add a listener to the Service for callbacks on add, remove, and update events
	// for data to the Service.
	virtual void AddListener(ServiceListener<PV01<T>>* listener)
	{
		listeners.push_back(listener);
	}

	// Get all listeners on the Service.
	virtual const vector< ServiceListener<PV01<T>>* >& GetListeners() const
	{
		return listeners;
	}

	virtual RiskPositionListener<T>* GetPositionListener()
	{
		return position_listener;
	}

	// Add a position that the service will risk
	void AddPosition(Position<T>& position)
	{
		//std::cout << position.GetAggregatePosition() << std::endl;
		T product = position.GetProduct();
		string id = product.GetProductId();
		double pv01_value = CaluculatePV01(id);
		long quantity = position.GetAggregatePosition();
		PV01<T> pv01(product, pv01_value, quantity);
		OnMessage(pv01);
	}

	// Get the bucketed risk for the bucket sector
	const PV01< BucketedSector<T> >& GetBucketedRisk(const BucketedSector<T>& sector) const
	{
		BucketedSector<T> product = sector;
		double pv01_value = 0;
		double quantity = 0;
		vector<T>& products = sector.GetProducts();
		for (auto p = products.begin(); p != products.end(); p++)
		{
			string id = p->GetProductId();
			pv01_value += pv01map[id].GetPV01() * pv01map[id].GetQuantity();
			quantity += pv01map[id].GetQuantity;
		}
		auto pv01 = PV01<BucketedSector<T>>(product, pv01_value, quantity);
		return pv01;
	}

};

template<typename T>
PV01<T>::PV01(const T &_product, double _pv01, long _quantity) :
  product(_product)
{
  pv01 = _pv01;
  quantity = _quantity;
}

template<typename T>
BucketedSector<T>::BucketedSector(const vector<T>& _products, string _name) :
  products(_products)
{
  name = _name;
}

template<typename T>
const vector<T>& BucketedSector<T>::GetProducts() const
{
  return products;
}

template<typename T>
const string& BucketedSector<T>::GetName() const
{
  return name;
}


/*Risk to position listener
*/

template<typename T>
class RiskPositionListener :public ServiceListener<Position<T>>
{
private:
	RiskService<T>* service;

public:
	//Ctor and Dtor
	RiskPositionListener() {}
	RiskPositionListener(RiskService<T>* s)
	{
		service = s;
	}
	~RiskPositionListener() = default;

	// Listener callback to process an add event to the Service
	void ProcessAdd(Position<T>& data)
	{
		service->AddPosition(data);
	}

	// Listener callback to process a remove event to the Service
	void ProcessRemove(Position<T>& _data){}

	// Listener callback to process an update event to the Service
	void ProcessUpdate(Position<T>& _data){}
};



#endif
