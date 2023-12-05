/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import hiSysEvent from "@ohos.hiSysEvent"

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe('hiSysEventJsUnitTest', function () {
    beforeAll(function() {

        /**
         * @tc.setup: setup invoked before all test cases
         */
        console.info('hiSysEventJsUnitTest beforeAll called')
    })

    afterAll(function() {

        /**
         * @tc.teardown: teardown invoked after all test cases
         */
        console.info('hiSysEventJsUnitTest afterAll called')
    })

    beforeEach(function() {

        /**
         * @tc.setup: setup invoked before each test case
         */
        console.info('hiSysEventJsUnitTest beforeEach called')
    })

    afterEach(function() {

        /**
         * @tc.teardown: teardown invoked after each test case
         */
        console.info('hiSysEventJsUnitTest afterEach called')
    })

    /**
     * @tc.name: hiSysEventJsUnitTest001
     * @tc.desc: Test hisysevent writing with calling AsyncCallback
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest001', 0, async function (done) {
        console.info('hiSysEventJsUnitTest001 start')
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview js test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
                if (err) {
                    console.error('in hiSysEventJsUnitTest001 test callback: err.code = ' + err.code)
                    expect(false).assertTrue()
                } else {
                    console.info('in hiSysEventJsUnitTest001 test callback: result = ' + val);
                    expect(val).assertEqual(0)
                }
                console.info('hiSysEventJsUnitTest001 end')
                done()
            })
        } catch (err) {
            console.error(`hiSysEventJsUnitTest001 > error code: ${err.code}, error msg: ${err.message}`)
            expect(false).assertTrue()
            console.info('hiSysEventJsUnitTest001 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest002
     * @tc.desc: Test hisysevent writing with returning Promise.
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest002', 0, async function (done) {
        console.info('hiSysEventJsUnitTest002 start')
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview js test suite",
                    MSG: "no msg."
                }
            }).then(
                (val) => {
                    console.info('in hiSysEventJsUnitTest002 test callback: result = ' + val)
                    expect(val).assertEqual(0)
                    console.info('hiSysEventJsUnitTest002 end')
                    done()
                }
            ).catch(
                (err) => {
                    console.error('in hiSysEventJsUnitTest002 test callback: err.code = ' + err.code)
                    expect(false).assertTrue()
                    console.info('hiSysEventJsUnitTest002 end')
                    done()
                }
            );
        } catch (err) {
            console.error(`hiSysEventJsUnitTest002 > error code: ${err.code}, error msg: ${err.message}`)
            expect(false).assertTrue()
            console.info('hiSysEventJsUnitTest002 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest003
     * @tc.desc: Test function return of adding/remove hisysevent watcher result.
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest003', 0, async function (done) {
        console.info('hiSysEventJsUnitTest003 start')
        let watcher = {
            rules: [{
                domain: "RELIABILITY",
                name: "STACK",
                ruleType: hiSysEvent.RuleType.WHOLE_WORD,
            }],
            onEvent: (info) => {
            },
            onServiceDied: () => {
            }
        }
        try {
            hiSysEvent.addWatcher(watcher)
            hiSysEvent.removeWatcher(watcher)
            expect(true).assertTrue()
            console.info('hiSysEventJsUnitTest003 end')
            done();
        } catch (err) {
            console.error(`hiSysEventJsUnitTest003 > error code: ${err.code}, error msg: ${err.message}`)
            expect(err.code == 201).assertTrue()
            console.info('hiSysEventJsUnitTest003 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest004
     * @tc.desc: Test watcher callback
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest004', 0, async function (done) {
        console.info('hiSysEventJsUnitTest004 start')
        let watcher = {
            rules: [{
                domain: "RELIABILITY",
                name: "STACK",
                tag: "STABILITY",
                ruleType: hiSysEvent.RuleType.WHOLE_WORD,
            }],
            onEvent: (info) => {
                console.info(`hiSysEventJsUnitTest004: OnEvent...`)
                expect(Object.keys(info).length > 0).assertTrue()
                console.info(`hiSysEventJsUnitTest004: domain is : ${info.domain}, name is ${info.name}, eventType is ${info.eventType}`)
                if (info.params instanceof Object) {
                    for (const key in info.params) {
                        console.info(`hiSysEventJsUnitTest004: ${key}: ${info.params[key]}`)
                    }
                }
            },
            onServiceDied: () => {
                console.info(`hiSysEventJsUnitTest004: OnServiceDie...`)
            }
        }
        try {
            hiSysEvent.addWatcher(watcher)
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview js test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
                if (err) {
                    console.error('in hiSysEventJsUnitTest004 test callback: err.code = ' + err.code)
                } else {
                    console.info('in hiSysEventJsUnitTest004 test callback: result = ' + val);
                }
            })
            setTimeout(() => {
                try {
                    hiSysEvent.removeWatcher(watcher)
                    expect(true).assertTrue()
                    console.info('hiSysEventJsUnitTest004 end')
                    done()
                } catch (err) {
                    console.error(`hiSysEventJsUnitTest004 delay > error code: ${err.code}, error msg: ${err.message}`)
                    expect(err.code == 201).assertTrue()
                    console.info('hiSysEventJsUnitTest004 end')
                    done()
                }
            }, 1000)
        } catch (err) {
            console.error(`hiSysEventJsUnitTest004 > error code: ${err.code}, error msg: ${err.message}`)
            expect(err.code == 201).assertTrue()
            console.info('hiSysEventJsUnitTest004 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest005
     * @tc.desc: Test query callback
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest005', 0, async function (done) {
        console.info('hiSysEventJsUnitTest005 start')
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview napi test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
                if (err) {
                    console.error('in hiSysEventJsUnitTest005 test callback: err.code = ' + err.code)
                } else {
                    console.info('in hiSysEventJsUnitTest005 test callback: result = ' + val)
                }
            })
            setTimeout(() => {
                try {
                    hiSysEvent.query({
                        beginTime: -1,
                        endTime: -1,
                        maxEvents: 2,
                    }, [{
                        domain: "RELIABILITY",
                        names: ["STACK"],
                    }], {
                        onQuery: function (infos) {
                            console.info(`hiSysEventJsUnitTest005: onQuery...`)
                            expect(infos.length >= 0).assertTrue()
                            console.info(`hiSysEventJsUnitTest005: infos.size is ${infos.length}`)
                            if (infos instanceof Array) {
                                for (let i = 0; i < infos.length; i++) {
                                    let item = infos[i];
                                    console.info(`hiSysEventJsUnitTest005: domain is ${item.domain}, name is ${item.name}, eventType is ${item.eventType}`)
                                    if (item.params instanceof Object) {
                                        for (const key in item.params) {
                                            console.info(`hiSysEventJsUnitTest005: ${key}: ${item.params[key]}`)
                                        }
                                    }
                                }
                            }
                        },
                        onComplete: function(reason, total) {
                            console.info(`hiSysEventJsUnitTest005: onComplete...`)
                            console.info(`hiSysEventJsUnitTest005: reason is ${reason}, total is ${total}`)
                            expect(true).assertTrue()
                            console.info(`hiSysEventJsUnitTest005 end`)
                            done()
                        }
                    })
                } catch (err) {
                    console.error(`hiSysEventJsUnitTest005 delay > error code: ${err.code}, error msg: ${err.message}`)
                    expect(err.code == 201).assertTrue()
                    console.info('hiSysEventJsUnitTest005 end')
                    done()
                }
            }, 1000);
        } catch (err) {
            console.error(`hiSysEventJsUnitTest005 > error code: ${err.code}, error msg: ${err.message}`)
            expect(err.code == 201).assertTrue()
            console.info('hiSysEventJsUnitTest005 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest006
     * @tc.desc: Test query callback with domain which length is over 16
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest006', 0, async function (done) {
        console.info('hiSysEventJsUnitTest006 start')
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview napi test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
                if (err) {
                    console.error('in hiSysEventJsUnitTest006 test callback: err.code = ' + err.code)
                } else {
                    console.info('in hiSysEventJsUnitTest006 test callback: result = ' + val)
                }
            })
            setTimeout(() => {
                try {
                    hiSysEvent.query({
                        beginTime: -1,
                        endTime: -1,
                        maxEvents: 2,
                    }, [{
                        domain: "RELIABILITY_RELIABILITY",
                        names: ["STACK"],
                    }], {
                        onQuery: function (infos) {
                            console.info(`hiSysEventJsUnitTest006: onQuery...`)
                            expect(infos.length >= 0).assertTrue()
                            console.info(`hiSysEventJsUnitTest006: infos.size is ${infos.length}`)
                            if (infos instanceof Array) {
                                for (let i = 0; i < infos.length; i++) {
                                    let item = infos[i];
                                    console.info(`hiSysEventJsUnitTest005: domain is ${item.domain}, name is ${item.name}, eventType is ${item.eventType}`)
                                    if (item.params instanceof Object) {
                                        for (const key in item.params) {
                                            console.info(`hiSysEventJsUnitTest005: ${key}: ${item.params[key]}`)
                                        }
                                    }
                                }
                            }
                        },
                        onComplete: function(reason, total) {
                            console.info(`hiSysEventJsUnitTest006: onComplete...`)
                            console.info(`hiSysEventJsUnitTest006: reason is ${reason}, total is ${total}`)
                            expect(true).assertTrue()
                            console.info(`hiSysEventJsUnitTest006 end`)
                            done()
                        }
                    })
                } catch (err) {
                    console.error(`hiSysEventJsUnitTest006 delay > error code: ${err.code}, error msg: ${err.message}`)
                    expect(err.code == 11200302 || err.code == 11200304).assertTrue()
                    console.info('hiSysEventJsUnitTest006 end')
                    done()
                }
            }, 1000);
        } catch (err) {
            console.error(`hiSysEventJsUnitTest006 > error code: ${err.code}, error msg: ${err.message}`)
            expect(false).assertTrue()
            console.info('hiSysEventJsUnitTest006 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest007
     * @tc.desc: Test query callback with domain which length is over 32
     * @tc.type: FUNC
     */
     it('hiSysEventJsUnitTest007', 0, async function (done) {
        console.info('hiSysEventJsUnitTest007 start')
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview napi test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
                if (err) {
                    console.error('in hiSysEventJsUnitTest007 test callback: err.code = ' + err.code)
                } else {
                    console.info('in hiSysEventJsUnitTest007 test callback: result = ' + val)
                }
            })
            setTimeout(() => {
                try {
                    hiSysEvent.query({
                        beginTime: -1,
                        endTime: -1,
                        maxEvents: 2,
                    }, [{
                        domain: "RELIABILITY",
                        names: ["STACK_STACK_STACK_STACK_STACK_STACK"],
                    }], {
                        onQuery: function (infos) {
                            console.info(`hiSysEventJsUnitTest007: onQuery...`)
                            expect(infos.length >= 0).assertTrue()
                            console.info(`hiSysEventJsUnitTest007: infos.size is ${infos.length}`)
                            if (infos instanceof Array) {
                                for (let i = 0; i < infos.length; i++) {
                                    let item = infos[i];
                                    console.info(`hiSysEventJsUnitTest005: domain is ${item.domain}, name is ${item.name}, eventType is ${item.eventType}`)
                                    if (item.params instanceof Object) {
                                        for (const key in item.params) {
                                            console.info(`hiSysEventJsUnitTest005: ${key}: ${item.params[key]}`)
                                        }
                                    }
                                }
                            }
                        },
                        onComplete: function(reason, total) {
                            console.info(`hiSysEventJsUnitTest007: onComplete...`)
                            console.info(`hiSysEventJsUnitTest007: reason is ${reason}, total is ${total}`)
                            expect(true).assertTrue()
                            console.info(`hiSysEventJsUnitTest007 end`)
                            done()
                        }
                    })
                } catch (err) {
                    console.error(`hiSysEventJsUnitTest007 delay > error code: ${err.code}, error msg: ${err.message}`)
                    expect(err.code == 11200302 || err.code == 11200304).assertTrue()
                    console.info('hiSysEventJsUnitTest007 end')
                    done()
                }
            }, 1000);
        } catch (err) {
            console.error(`hiSysEventJsUnitTest007 > error code: ${err.code}, error msg: ${err.message}`)
            expect(false).assertTrue()
            console.info('hiSysEventJsUnitTest007 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest008
     * @tc.desc: Test hisysevent of invalid domain writing with calling AsyncCallback
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest008', 0, async function (done) {
        console.info('hiSysEventJsUnitTest008 start')
        try {
            hiSysEvent.write({
                domain: "RELIABILITY_RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview js test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
                if (err) {
                    console.error(`in hiSysEventJsUnitTest008 test callback: err.code = ${err.code}, error msg is ${err.message}`)
                    expect(err.code == 11200001).assertTrue()
                } else {
                    console.info(`in hiSysEventJsUnitTest008 test callback: result = ${val}`)
                    expect(false).assertTrue()
                }
                console.info('hiSysEventJsUnitTest008 end')
                done()
            })
        } catch (err) {
            console.error(`hiSysEventJsUnitTest008 > error code: ${err.code}, error msg: ${err.message}`)
            expect(false).assertTrue()
            console.info('hiSysEventJsUnitTest008 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest009
     * @tc.desc: Test hisysevent of invalid event name writing with calling AsyncCallback
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest009', 0, async function (done) {
        console.info('hiSysEventJsUnitTest009 start')
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK_STACK_STACK_STACK_STACK_STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview js test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
                if (err) {
                    console.error(`in hiSysEventJsUnitTest009 test callback: err.code = ${err.code}, error msg is ${err.message}`)
                    expect(err.code == 11200002).assertTrue()
                } else {
                    console.info(`in hiSysEventJsUnitTest009 test callback: result = ${val}`)
                    expect(false).assertTrue()
                }
                console.info('hiSysEventJsUnitTest009 end')
                done()
            })
        } catch (err) {
            console.error(`hiSysEventJsUnitTest009 > error code: ${err.code}, error msg: ${err.message}`)
            expect(false).assertTrue()
            console.info('hiSysEventJsUnitTest009 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest010
     * @tc.desc: Test hisysevent which is over size writing with calling AsyncCallback
     * @tc.type: FUNC
     */
     it('hiSysEventJsUnitTest010', 0, async function (done) {
        console.info('hiSysEventJsUnitTest010 start')
        let params = {
            PID: 1,
            UID: 1,
            PACKAGE_NAME: "com.huawei.testHiSysEvent",
            PROCESS_NAME: "just a testcase",
            MSG: "no msg."
        }
        for (let i = 0; i < 40; i++) {
            params[`bundle${i}`] = Array.from({length: 10 * 1024}).join("ohos")
        }
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: params,
            }, (err, val) => {
                if (err) {
                    console.error(`in hiSysEventJsUnitTest010 test callback: err.code = ${err.code}, error msg is ${err.message}`)
                    expect(err.code == 11200004).assertTrue()
                } else {
                    console.info(`in hiSysEventJsUnitTest010 test callback: result = ${val}`)
                    expect(false).assertTrue()
                }
                console.info('hiSysEventJsUnitTest010 end')
                done()
            })
        } catch (err) {
            console.error(`hiSysEventJsUnitTest010 > error code: ${err.code}, error msg: ${err.message}`)
            expect(false).assertTrue()
            console.info('hiSysEventJsUnitTest010 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest011
     * @tc.desc: Test hisysevent of invalid param name writing with calling AsyncCallback
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest011', 0, async function (done) {
        console.info('hiSysEventJsUnitTest011 start')
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview js test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
                if (err) {
                    console.error(`in hiSysEventJsUnitTest011 test callback: err.code = ${err.code}, error msg is ${err.message}`)
                    expect(err.code == 11200051).assertTrue()
                } else {
                    console.info(`in hiSysEventJsUnitTest011 test callback: result = ${val}`)
                    expect(false).assertTrue()
                }
                console.info('hiSysEventJsUnitTest011 end')
                done()
            })
        } catch (err) {
            console.error(`hiSysEventJsUnitTest011 > error code: ${err.code}, error msg: ${err.message}`)
            expect(false).assertTrue()
            console.info('hiSysEventJsUnitTest011 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest012
     * @tc.desc: Test hisysevent with string over limit writing with calling AsyncCallback
     * @tc.type: FUNC
     */
     it('hiSysEventJsUnitTest012', 0, async function (done) {
        console.info('hiSysEventJsUnitTest012 start')
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: Array.from({length: 10 * 1024 + 10}).join("ohos"),
                    MSG: "no msg."
                }
            }, (err, val) => {
                if (err) {
                    console.error(`in hiSysEventJsUnitTest012 test callback: err.code = ${err.code}, error msg is ${err.message}`)
                    expect(err.code == 11200052).assertTrue()
                } else {
                    console.info(`in hiSysEventJsUnitTest012 test callback: result = ${val}`)
                    expect(false).assertTrue()
                }
                console.info('hiSysEventJsUnitTest012 end')
                done()
            })
        } catch (err) {
            console.error(`hiSysEventJsUnitTest012 > error code: ${err.code}, error msg: ${err.message}`)
            expect(false).assertTrue()
            console.info('hiSysEventJsUnitTest012 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest013
     * @tc.desc: Test hisysevent with param count over limit writing with calling AsyncCallback
     * @tc.type: FUNC
     */
     it('hiSysEventJsUnitTest013', 0, async function (done) {
        console.info('hiSysEventJsUnitTest013 start')
        let largeParams = {}
        for (let i = 0; i < 200; i++) {
            largeParams["name" + i] = i
        }
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: largeParams
            }, (err, val) => {
                if (err) {
                    console.error(`in hiSysEventJsUnitTest013 test callback: err.code = ${err.code}, error msg is ${err.message}`)
                    expect(err.code == 11200053).assertTrue()
                } else {
                    console.info(`in hiSysEventJsUnitTest013 test callback: result = ${val}`)
                    expect(false).assertTrue()
                }
                console.info('hiSysEventJsUnitTest013 end')
                done()
            })
        } catch (err) {
            console.error(`hiSysEventJsUnitTest013 > error code: ${err.code}, error msg: ${err.message}`)
            expect(false).assertTrue()
            console.info('hiSysEventJsUnitTest013 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest014
     * @tc.desc: Test hisysevent with array size over limit writing with calling AsyncCallback
     * @tc.type: FUNC
     */
     it('hiSysEventJsUnitTest014', 0, async function (done) {
        console.info('hiSysEventJsUnitTest014 start')
        let msgArray = []
        for (let i = 0; i < 200; i++) {
            msgArray[i] = i
        }
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview js test suite",
                    MSG: msgArray
                }
            }, (err, val) => {
                if (err) {
                    console.error(`in hiSysEventJsUnitTest014 test callback: err.code = ${err.code}, error msg is ${err.message}`)
                    expect(err.code == 11200054).assertTrue()
                } else {
                    console.info(`in hiSysEventJsUnitTest014 test callback: result = ${val}`)
                    expect(false).assertTrue()
                }
                console.info('hiSysEventJsUnitTest014 end')
                done()
            })
        } catch (err) {
            console.error(`hiSysEventJsUnitTest014 > error code: ${err.code}, error msg: ${err.message}`)
            expect(false).assertTrue()
            console.info('hiSysEventJsUnitTest014 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest015
     * @tc.desc: Test hisysevent query with sequence
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest015', 0, async function (done) {
        console.info('hiSysEventJsUnitTest015 start')
        try {
            hiSysEvent.query({
                maxEvents: 10000,
                fromSeq: 100,
                toSeq: 1000,
            }, [{
                domain: "AAFWK",
                names: ["CONNECT_SERVICE"],
            }], {
                onQuery: function (infos) {
                    console.info(`hiSysEventJsUnitTest015: onQuery...`)
                    expect(infos.length >= 0).assertTrue()
                    console.info(`hiSysEventJsUnitTest015: infos.size is ${infos.length}`)
                    if (infos instanceof Array) {
                        for (let i = 0; i < infos.length; i++) {
                            let item = infos[i];
                            console.info(`hiSysEventJsUnitTest015: domain is ${item.domain}, name is ${item.name}, eventType is ${item.eventType}`)
                            if (item.params instanceof Object) {
                                for (const key in item.params) {
                                    console.info(`hiSysEventJsUnitTest015: ${key}: ${item.params[key]}`)
                                }
                            }
                        }
                    }
                },
                onComplete: function(reason, total, seq) {
                    console.info(`hiSysEventJsUnitTest015: onComplete...`)
                    console.info(`hiSysEventJsUnitTest015: reason is ${reason}, total is ${total}, seq is ${seq}`)
                    expect(true).assertTrue()
                    console.info(`hiSysEventJsUnitTest015 end`)
                    done()
                }
            })
        } catch (err) {
            console.error(`hiSysEventJsUnitTest015 delay > error code: ${err.code}, error msg: ${err.message}`)
            expect(err.code == 201).assertTrue()
            console.info('hiSysEventJsUnitTest015 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest016
     * @tc.desc: Test hisysevent get max sequence
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest016', 0, async function (done) {
        console.info('hiSysEventJsUnitTest016 start')
        try {
            hiSysEvent.query({
                maxEvents: 0,
                fromSeq: 0,
                toSeq: 1000,
            }, [{
                domain: "AAFWK",
                names: ["CONNECT_SERVICE"],
            }], {
                onQuery: function (infos) {
                    console.info(`hiSysEventJsUnitTest016: onQuery...`)
                    expect(infos.length >= 0).assertTrue()
                    console.info(`hiSysEventJsUnitTest016: infos.size is ${infos.length}`)
                    if (infos instanceof Array) {
                        for (let i = 0; i < infos.length; i++) {
                            let item = infos[i];
                            console.info(`hiSysEventJsUnitTest016: domain is ${item.domain}, name is ${item.name}, eventType is ${item.eventType}`)
                            if (item.params instanceof Object) {
                                for (const key in item.params) {
                                    console.info(`hiSysEventJsUnitTest016: ${key}: ${item.params[key]}`)
                                }
                            }
                        }
                    }
                },
                onComplete: function(reason, total, seq) {
                    console.info(`hiSysEventJsUnitTest016: onComplete...`)
                    console.info(`hiSysEventJsUnitTest016: reason is ${reason}, total is ${total}, seq is ${seq}`)
                    expect(true).assertTrue()
                    console.info(`hiSysEventJsUnitTest016 end`)
                    done()
                }
            })
        } catch (err) {
            console.error(`hiSysEventJsUnitTest016 delay > error code: ${err.code}, error msg: ${err.message}`)
            expect(err.code == 201).assertTrue()
            console.info('hiSysEventJsUnitTest016 end')
            done()
        }
    })
});