
/*
 * $Id: DelayVector.cc,v 1.5 2003/03/10 20:12:43 robertc Exp $
 *
 * DEBUG: section 77    Delay Pools
 * AUTHOR: Robert Collins <robertc@squid-cache.org>
 * Based upon original delay pools code by
 *   David Luyer <david@luyer.net>
 *
 * SQUID Web Proxy Cache          http://www.squid-cache.org/
 * ----------------------------------------------------------
 *
 *  Squid is the result of efforts by numerous individuals from
 *  the Internet community; see the CONTRIBUTORS file for full
 *  details.   Many organizations have provided support for Squid's
 *  development; see the SPONSORS file for full details.  Squid is
 *  Copyrighted (C) 2001 by the Regents of the University of
 *  California; see the COPYRIGHT file for full details.  Squid
 *  incorporates software developed and/or copyrighted by other
 *  sources; see the CREDITS file for full details.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 *
 * Copyright (c) 2003, Robert Collins <robertc@squid-cache.org>
 */

#include "config.h"

#if DELAY_POOLS
#include "squid.h"
#include "DelayVector.h"
#include "CommRead.h"

void *
DelayVector::operator new(size_t size)
{
    DelayPools::MemoryUsed += sizeof (DelayVector);
    return ::operator new (size);
}

void
DelayVector::operator delete (void *address)
{
    DelayPools::MemoryUsed -= sizeof (DelayVector);
    ::operator delete (address);
}

void
DelayVector::deleteSelf() const
{
    delete this;
}

DelayVector::~DelayVector()
{}

void
DelayVector::stats(StoreEntry * sentry)
{
    iterator pos = pools.begin();

    while (pos != pools.end()) {
        (*pos)->stats(sentry);
        ++pos;
    }
}

void
DelayVector::dump(StoreEntry *entry) const
{
    const_iterator pos = pools.begin();

    while (pos != pools.end()) {
        (*pos)->dump(entry);
        ++pos;
    }
}

void
DelayVector::update(int incr)
{
    iterator pos = pools.begin();

    while (pos != pools.end()) {
        (*pos)->update(incr);
        ++pos;
    }

    kickReads();
}

void
DelayVector::parse()
{
    iterator pos = pools.begin();

    while (pos != pools.end()) {
        (*pos)->parse();
        ++pos;
    }
}

DelayIdComposite::Pointer

DelayVector::id(struct in_addr &src_addr, AuthUserRequest *authUser)
{
    return new Id(this, src_addr, authUser);
}

void
DelayVector::push_back(CompositePoolNode::Pointer aNode)
{
    pools.push_back(aNode);
}

void *
DelayVector::Id::operator new(size_t size)
{
    DelayPools::MemoryUsed += sizeof (Id);
    return ::operator new (size);
}

void
DelayVector::Id::operator delete (void *address)
{
    DelayPools::MemoryUsed -= sizeof (Id);
    ::operator delete (address);
}

void
DelayVector::Id::deleteSelf() const
{
    delete this;
}

DelayVector::Id::Id(DelayVector::Pointer aDelayVector,struct in_addr &src_addr, AuthUserRequest *authUser) : theVector(aDelayVector)
{
    debug(77,3)("DelayVector::Id::Id\n");
    DelayVector::iterator pos = theVector->pools.begin();

    while (pos != theVector->pools.end())
    {
        ids.push_back ((*pos)->id (src_addr, authUser));
        ++pos;
    }
}

DelayVector::Id::~Id()
{
    debug(77,3)("DelayVector::Id::~Id\n");
}

int
DelayVector::Id::bytesWanted (int minimum, int maximum) const
{
    int nbytes = maximum;
    const_iterator pos = ids.begin();

    while (pos != ids.end()) {
        nbytes = min (nbytes, (*pos)->bytesWanted(minimum, nbytes));
        ++pos;
    }

    nbytes = max(minimum, nbytes);
    return nbytes;
}

void
DelayVector::Id::bytesIn(int qty)
{
    iterator pos = ids.begin();

    while (pos != ids.end()) {
        (*pos)->bytesIn(qty);
        ++pos;
    }

    theVector->kickReads();
}

void
DelayVector::Id::delayRead(DeferredRead const &aRead)
{
    theVector->delayRead(aRead);
}

#endif
