/**
 * streamingservice.hpp
 * Defines the data types and Service for price streams.
 *
 * @author Breman Thuraisingham
 */
#ifndef STREAMING_SERVICE_HPP
#define STREAMING_SERVICE_HPP

#include "soa.hpp"
#include "marketdataservice.hpp"
#include "algostreamingservice.hpp"

/**
 * A price stream order with price and quantity (visible and hidden)
 */
/*
class PriceStreamOrder
{

public:

  // ctor for an order
  PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side);

  // The side on this order
  PricingSide GetSide() const;

  // Get the price on this order
  double GetPrice() const;

  // Get the visible quantity on this order
  long GetVisibleQuantity() const;

  // Get the hidden quantity on this order
  long GetHiddenQuantity() const;

private:
  double price;
  long visibleQuantity;
  long hiddenQuantity;
  PricingSide side;

};
*/

/**
 * Price Stream with a two-way market.
 * Type T is the product type.
 */
/*
template<typename T>
class PriceStream
{

public:

  // ctor
  PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder);

  // Get the product
  const T& GetProduct() const;

  // Get the bid order
  const PriceStreamOrder& GetBidOrder() const;

  // Get the offer order
  const PriceStreamOrder& GetOfferOrder() const;

private:
  T product;
  PriceStreamOrder bidOrder;
  PriceStreamOrder offerOrder;

};
*/

/*
* Declaration
*/
template<typename T>
class StreamingAlgoStreamingListener;


/**
 * Streaming service to publish two-way prices.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class StreamingService : public Service<string,PriceStream <T> >
{
private:
	map<string, PriceStream<T>> pricestreammap;
	vector<ServiceListener<PriceStream<T>>*> listeners;
	StreamingAlgoStreamingListener<T>* AlgoStreamingListener;

public:
	//Ctor and Dtor
	StreamingService()
	{
		pricestreammap = map<string, PriceStream<T>>();
		listeners = vector<ServiceListener<PriceStream<T>>*>();
		AlgoStreamingListener = new StreamingAlgoStreamingListener<T>(this);
	}
	~StreamingService() = default;
	// Get data on our service given a key
	PriceStream<T>& GetData(string key)
	{
		return pricestreammap[key];
	}

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(PriceStream<T>& data)
	{
		string key = data.GetProduct().GetProductId();
		pricestreammap[key] = data;
		//Notify all the listeners
		for (auto i = listeners.begin(); i != listeners.end(); i++)
		{
			(*i)->ProcessAdd(data);
		}
	}

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<PriceStream<T>>* listener)
	{
		listeners.push_back(listener);
	}

	// Get all listeners on the Service
	const vector<ServiceListener<PriceStream<T>>*>& GetListeners() const
	{
		return listeners;
	}

	// Get the listener of the service
	StreamingAlgoStreamingListener<T>* GetAlgoStreamingListener()
	{
		return AlgoStreamingListener;
	}

  // Publish two-way prices
	virtual void PublishPrice(PriceStream<T>& pricestream)
	{
		OnMessage(pricestream);
	}

};
/*

PriceStreamOrder::PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side)
{
  price = _price;
  visibleQuantity = _visibleQuantity;
  hiddenQuantity = _hiddenQuantity;
  side = _side;
}

double PriceStreamOrder::GetPrice() const
{
  return price;
}

long PriceStreamOrder::GetVisibleQuantity() const
{
  return visibleQuantity;
}

long PriceStreamOrder::GetHiddenQuantity() const
{
  return hiddenQuantity;
}

template<typename T>
PriceStream<T>::PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder) :
  product(_product), bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

template<typename T>
const T& PriceStream<T>::GetProduct() const
{
  return product;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetBidOrder() const
{
  return bidOrder;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetOfferOrder() const
{
  return offerOrder;
}
*/



/*
* Streaming service listener, to algo stream service
*/
template<typename T>
class StreamingAlgoStreamingListener :public ServiceListener<AlgoStream<T>>
{
private:
	StreamingService<T>* service;

public:
	//Ctor and Dtor
	StreamingAlgoStreamingListener(StreamingService<T>* s)
	{
		service = s;
	}
	~StreamingAlgoStreamingListener() = default;

	// Listener callback to process an add event to the Service
	void ProcessAdd(AlgoStream<T>& data)
	{
		service->PublishPrice(*(data.GetPriceStream()));
	}

	// Listener callback to process a remove event to the Service
	void ProcessRemove(AlgoStream<T>& data){}

	// Listener callback to process an update event to the Service
	void ProcessUpdate(AlgoStream<T>& data){}
};


#endif
