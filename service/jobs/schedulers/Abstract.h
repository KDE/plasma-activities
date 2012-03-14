/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef JOBS_SCHEDULER_ABSTRACT_H_
#define JOBS_SCHEDULER_ABSTRACT_H_

#include <jobs/Job.h>
#include <jobs/JobFactory.h>

namespace Jobs {
namespace Schedulers {

/**
 * Abstract
 */
class Abstract: public Job {
    Q_OBJECT

public:
    Abstract(QObject * parent = 0);
    virtual ~Abstract();

    virtual void start();

protected:
    bool startJob(int index);
    virtual void jobFinished(int result) = 0;

    int lastJobStarted() const;
    int jobCount() const;
    bool hasJob(int index) const;

    void addJob(JobFactory * job);
    void addJob(Job * job);

    void returnResult(int result);

private:
    Abstract(const Abstract & original);
    Abstract & operator = (const Abstract & original);

    class Private;
    Private * const d;
    friend class Private;
};

// class Abstract

} // namespace Schedulers
} // namespace Jobs

#endif // JOBS_SCHEDULER_ABSTRACT_H_

