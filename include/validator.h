#ifndef VALIDATOR_H
#define VALIDATOR_H

class Validator
{
public:
   virtual bool validate(Variant &val) = 0;
};

class IntValidator : public Validator
{
public:
   IntValidator(int min, int max)
   {
      m_min = min;
      m_max = max;
   }

   virtual bool validate(Variant &val)
   {
      bool rv;
      if (val.toInt() >= m_min && val.toInt() < m_max)
      {
         rv = true;
      }
      else
      {
         rv = false;
      }
      return(rv);
   }

private:
   int   m_min;
   int   m_max;
};

class DoubleValidator : public Validator
{
public:
   DoubleValidator(double min, double max)
   {
      m_min = min;
      m_max = max;
   }

   virtual bool validate(Variant &val)
   {
      bool rv;
      if (val.toDouble() >= m_min && val.toDouble() < m_max)
      {
         rv = true;
      }
      else
      {
         rv = false;
      }
      return(rv);
   }

private:
   double   m_min;
   double   m_max;
};

#endif
