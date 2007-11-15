/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Tom Henderson <tomhend@u.washington.edu>
 * This code has been ported from ns-2 (queue/errmodel.{cc,h}
 */
#ifndef ERROR_MODEL_H
#define ERROR_MODEL_H

#include <list>
#include "ns3/object.h"
#include "ns3/component-manager.h"

namespace ns3 {

class Packet;
class RandomVariable;

/**
 * \brief General error model that can be used to corrupt packets
 *
 * This object is used to flag packets as being lost/errored or not.
 * It is part of the Object framework and can be aggregated to 
 * other ns3 objects and handled by the Ptr class.
 *
 * The main method is IsCorrupt(Packet& p) which returns true if 
 * the packet is to be corrupted according to the underlying model.
 * Depending on the error model, the packet itself may have its packet
 * data buffer errored or not, or side information may be returned to
 * the client in the form of a packet tag.
 * The object can have state (resettable by Reset()).  
 * The object can also be enabled and disabled via two public member functions.
 * 
 * Typical code (simplified) to use an ErrorModel may look something like 
 * this:
 * \code 
 * Ptr<ErrorModel> rem = Create<RateErrorModel> ();
 * rem->SetRandomVariable (UniformVariable ());
 * rem->SetRate (0.001);
 * ...
 * Packet p;
 * if (rem->IsCorrupt (p))
 *   {
 *     dropTrace(p);
 *   } else {
 *     Forward (p);
 *   }
 * \endcode
 *
 * Two practical error models, a ListErrorModel and a RateErrorModel,  
 * are currently implemented. 
 */
class ErrorModel : public Object
{
public:
  static const InterfaceId iid;
  /**
   * A factory method to generate a preconfigured default ErrorModel for use
   * \return an ErrorModel smart pointer that is the default ErrorModel 
   * type defined
   */
  static Ptr<ErrorModel> CreateDefault (void);
  
  ErrorModel ();
  virtual ~ErrorModel ();

 /**
  * \returns true if the Packet is to be considered as errored/corrupted
  * \param pkt Packet to apply error model to
  */
  bool IsCorrupt (Packet& pkt);
 /**
  * Reset any state associated with the error model
  */
  void Reset (void);
 /**
  * Enable the error model
  */
  void Enable (void);
 /**
  * Disable the error model
  */
  void Disable (void);
  /**
   * \return true if error model is enabled; false otherwise
   */
  bool IsEnabled (void) const;

protected:
  bool m_enable;

private:
  /*
   * These methods must be implemented by subclasses
   */
  virtual bool DoCorrupt (Packet&) = 0;
  virtual void DoReset (void) = 0;

};

enum ErrorUnit
  {   
    EU_BIT,
    EU_BYTE,
    EU_PKT
  };

/**
 * \brief Determine which packets are errored corresponding to an underlying
 * distribution, rate, and unit.
 *
 * This object is used to flag packets as being lost/errored or not.
 * The two parameters that govern the behavior are the rate (or
 * equivalently, the mean duration/spacing between errors), and the
 * unit (which may be per-bit, per-byte, and per-packet).
 * Users can optionally provide a RandomVariable object; the default
 * is to use a Uniform(0,1) distribution.

 * Reset() on this model will do nothing
 *
 * IsCorrupt() will not modify the packet data buffer
 */
class RateErrorModel : public ErrorModel
{
public:
  static const InterfaceId iid;
  static const ClassId cid;

  RateErrorModel ();
  virtual ~RateErrorModel ();

  /**
   * \returns the ErrorUnit being used by the underlying model
   */ 
  enum ErrorUnit GetUnit (void) const;
  /**
   * \param error_unit the ErrorUnit to be used by the underlying model
   */ 
  void SetUnit (enum ErrorUnit error_unit);

  /**
   * \returns the error rate being applied by the model
   */ 
  double GetRate (void) const;
  /**
   * \param rate the error rate to be used by the model
   */ 
  void SetRate (double rate);

  /**
   * \param ranvar A random variable distribution to generate random variates
   */ 
  void SetRandomVariable (const RandomVariable &ranvar);

private:
  virtual bool DoCorrupt (Packet& p);
  virtual bool DoCorruptPkt (Packet& p);
  virtual bool DoCorruptByte (Packet& p);
  virtual bool DoCorruptBit (Packet& p);
  virtual void DoReset (void);

  enum ErrorUnit m_unit;
  double m_rate;

  RandomVariable* m_ranvar;
};

/**
 * \brief Provide a list of Packet uids to corrupt
 *
 * This object is used to flag packets as being lost/errored or not.
 * A note on performance:  the list is assumed to be unordered, and
 * in general, Packet uids received may be unordered.  Therefore,
 * each call to IsCorrupt() will result in a walk of the list with
 * the present underlying implementation.
 * 
 * Note also that if one wants to target multiple packets from looking
 * at an (unerrored) trace file, the act of erroring a given packet may
 * cause subsequent packet uids to change.  For instance, suppose one wants 
 * to error packets 11 and 17 on a given device.  It may be that erroring
 * packet 11 will cause the subsequent uid stream to change and 17 may no
 * longer correspond to the second packet that one wants to lose.  Therefore,
 * be advised that it might take some trial and error to select the
 * right uids when multiple are provided.
 * 
 * Reset() on this model will clear the list
 *
 * IsCorrupt() will not modify the packet data buffer
 */
class ListErrorModel : public ErrorModel
{
public:
  static const InterfaceId iid;
  static const ClassId cid;
  ListErrorModel ();
  virtual ~ListErrorModel ();

  /**
   * \return a copy of the underlying list
   */
  std::list<uint32_t> GetList (void) const;
  /**
   * \param packetlist The list of packet uids to error.
   *
   * This method overwrites any previously provided list.
   */
  void SetList (const std::list<uint32_t> &packetlist);

private:
  virtual bool DoCorrupt (Packet& p);
  virtual void DoReset (void);

  typedef std::list<uint32_t> PacketList;
  typedef std::list<uint32_t>::const_iterator PacketListCI;

  PacketList m_packetList;
  
};


} //namespace ns3
#endif
