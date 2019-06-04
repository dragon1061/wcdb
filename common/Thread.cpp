/*
 * Tencent is pleased to support the open source community by making
 * WCDB available.
 *
 * Copyright (C) 2017 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *       https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <WCDB/Error.hpp>
#include <WCDB/Notifier.hpp>
#include <WCDB/Thread.hpp>

namespace WCDB {

#pragma mark - Initialize
Thread::Thread(pthread_t id) : m_id(id)
{
}

Thread::Thread() : Thread(nullptr)
{
}

Thread& Thread::operator=(const std::nullptr_t&)
{
    m_id = nullptr;
    return *this;
}

#pragma mark - Which
Thread Thread::current()
{
    return Thread(pthread_self());
}

bool Thread::isMain()
{
    return pthread_main_np() != 0;
}

bool Thread::isCurrentThread() const
{
    return pthreadEqual(m_id, pthread_self());
}

bool Thread::equal(const Thread& other) const
{
    return pthreadEqual(m_id, other.m_id);
}

bool Thread::operator==(const Thread& other) const
{
    return equal(other);
}

bool Thread::pthreadEqual(pthread_t left, pthread_t right)
{
    return pthread_equal(left, right) != 0;
}

#pragma mark - Name
void Thread::setName(const String& name)
{
    if (pthread_setname_np(name.c_str()) != 0) {
        setThreadedError();
    }
}

String Thread::getName()
{
    static constexpr const int s_maxLengthOfAllowedThreadName = 255;
    char name[s_maxLengthOfAllowedThreadName];
    memset(name, 0, s_maxLengthOfAllowedThreadName);
    if (pthread_getname_np(m_id, name, s_maxLengthOfAllowedThreadName) != 0) {
        setThreadedError();
    }
    return name;
}

uint64_t Thread::getIdentifier()
{
    uint64_t identifier = 0;
    if (pthread_threadid_np(m_id, &identifier) != 0) {
        setThreadedError();
    }
    return identifier;
}

#pragma mark - Error
void Thread::setThreadedError()
{
    Error error;
    error.level = Error::Level::Error;
    error.setSystemCode(errno, Error::Code::Error);
    Notifier::shared().notify(error);
    SharedThreadedErrorProne::setThreadedError(std::move(error));
}

} // namespace WCDB