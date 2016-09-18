/**
 * File:   Material.h
 * Author: Frantisek Nemec, xnemec61[at]stud.fit.vutbr.cz
 * Date: 2013-12-02
 */

#ifndef SKLADSUROVIN_H
#define	SKLADSUROVIN_H

#include <simlib.h>
#include <stdio.h>

class MaterialStore
{
    class MaterialGenerator : public Process 
    {
        private:

            double m_prepareTime; // Interval between generating new batch
            unsigned int m_batchSize;
            Store *m_store;

        public:

            MaterialGenerator(double time, unsigned int amount, Store *store) :
                m_prepareTime(time), m_batchSize(amount), m_store(store)
            {
            }

            void Behavior()
            {
                // Store initialization. At beginning store is empty.
                m_store->Enter(this, m_store->Free());

                while (1)
                {
                    if (m_store->Used() <= m_batchSize)
                    {
                        // Fill store to full capacity
                        m_store->Leave(m_store->Used());

                        // Store if full wait for someone to get some material.
                        // Then start preparing new batch.
                        WaitUntil(m_store->Used() > 0);
                    }
                    else
                    {
                        // Fill store
                        m_store->Leave(m_batchSize);
                    }

                    // Simulating material preparition
                    Wait(m_prepareTime);
                }
            }
    };

    private:

        Store * m_store;
        MaterialGenerator * m_generator;
        
        int m_usedMaterial;

    public:

        MaterialStore(double generateTime, unsigned  int generateAmount, unsigned int capacity) :
            m_usedMaterial(0)
        {
            m_store = new Store(capacity);
            m_generator = new MaterialGenerator(generateTime, generateAmount, m_store);
        }
        
        ~MaterialStore()
        {
            delete m_store;
            m_generator->Cancel();
        }

        void activate()
        {
            m_generator->Activate();
        }
        
        void get(Entity *entit, int amount)
        {
            m_store->Enter(entit, amount);
            m_usedMaterial += amount;
        }
        
        void output()
        {
            printf("Material used: %d\n", m_usedMaterial);
            m_store->Output();
        }
};

#endif	/* SKLADSUROVIN_H */

