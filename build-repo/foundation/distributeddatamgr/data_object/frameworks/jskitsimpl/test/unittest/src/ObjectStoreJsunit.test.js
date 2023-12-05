/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index';
import distributedObject from '@ohos.data.distributedDataObject';
import abilityAccessCtrl from '@ohos.abilityAccessCtrl';
import bundle from '@ohos.bundle';

var baseLine = 3000; //3 second
const TAG = "OBJECTSTORE_TEST";

function changeCallback(sessionId, changeData) {
    console.info("changeCallback start");
    if (changeData != null && changeData != undefined) {
        changeData.forEach(element => {
            console.info(TAG + "data changed !" + element);
        });
    }
    console.info("changeCallback end");
}

function changeCallback2(sessionId, changeData) {
    console.info("changeCallback2 start");
    if (changeData != null && changeData != undefined) {
        changeData.forEach(element => {
            console.info(TAG + "data changed !");
        });
    }
    console.info("changeCallback2 end");
}

function statusCallback1(sessionId, networkId, status) {
    console.info(TAG + "statusCallback1" + " " + sessionId);
    this.response += "\nstatus changed " + sessionId + " " + status + " " + networkId;
}

function statusCallback2(sessionId, networkId, status) {
    console.info(TAG + "statusCallback2" + " " + sessionId);
    this.response += "\nstatus changed " + sessionId + " " + status + " " + networkId;
}

function statusCallback3(sessionId, networkId, status) {
    console.info(TAG + "statusCallback3" + " " + sessionId);
    this.response += "\nstatus changed " + sessionId + " " + status + " " + networkId;
}

function statusCallback4(sessionId, networkId, status) {
    console.info(TAG + "statusCallback4" + " " + sessionId);
    expect("restored" == status).assertEqual(true);
}

const TIMEOUT = 1500;
const PERMISSION_USER_SET = 1;
const PERMISSION_USER_NAME = "ohos.permission.DISTRIBUTED_DATASYNC";
const CATCH_ERR = -1;
var tokenID = undefined;
async function grantPerm() {
    console.info("====grant Permission start====");
    var appInfo = await bundle.getApplicationInfo('com.OpenHarmony.app.test', 0, 100);
    tokenID = appInfo.accessTokenId;
    console.info("accessTokenId" + appInfo.accessTokenId + " bundleName:" + appInfo.bundleName);
    var atManager = abilityAccessCtrl.createAtManager();
    var result = await atManager.grantUserGrantedPermission(tokenID, PERMISSION_USER_NAME, PERMISSION_USER_SET);
    console.info("tokenId" + tokenID + " result:" + result);
    console.info("====grant Permission end====");
}
describe('objectStoreTest',function () {
    beforeAll(async function (done) {
        await grantPerm();
        done();
    })

    beforeEach(function () {
        console.info(TAG + 'beforeEach')
    })

    afterEach(function () {
        console.info(TAG + 'afterEach')
    })

    afterAll(function () {
        console.info(TAG + 'afterAll')
    })

    console.log(TAG + "*************Unit Test Begin*************");


    /**
     * @tc.name: testOn001
     * @tc.desc: object join session and on,object can receive callback when data has been changed
     * @tc.type: FUNC
     * @tc.require: I4H3LS
     */
    it('testOn001', 0, function () {
        console.log(TAG + "************* testOn001 start *************");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);
        g_object.setSessionId("session1");
        expect("session1" == g_object.__sessionId).assertEqual(true);
        console.info(TAG + " start call watch change");
        g_object.on("change", function (sessionId, changeData) {
            console.info("testOn001 callback start.");
            if (changeData != null && changeData != undefined) {
                changeData.forEach(element => {
                    console.info(TAG + "data changed !" + element);
                });
            }
            console.info("testOn001 callback end.");
        });

        if (g_object != undefined && g_object != null) {
            g_object.name = "jack1";
            g_object.age = 19;
            g_object.isVis = true;
            expect(g_object.name == "jack1").assertEqual(true);
            expect(g_object.age == 19).assertEqual(true);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        console.log(TAG + "************* testOn001 end *************");
        g_object.setSessionId("");
    })

    /**
     * @tc.name: testOn002
     * @tc.desc object join session and no on,obejct can not receive callback when data has been changed
     * @tc.type: FUNC
     * @tc.require: I4H3LS
     */
    it('testOn002', 0, function () {
        console.log(TAG + "************* testOn002 start *************");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);
        g_object.setSessionId("session2");
        expect("session2" == g_object.__sessionId).assertEqual(true);
        if (g_object != undefined && g_object != null) {
            g_object.name = "jack1";
            g_object.age = 19;
            g_object.isVis = true;
            expect(g_object.name == "jack1").assertEqual(true);
            expect(g_object.age == 19).assertEqual(true);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        console.log(TAG + "************* testOn002 end *************");
        g_object.setSessionId("");
    })

    /**
     * @tc.name: testOn003
     * @tc.desc: object join session and on,then object change data twice,object can receive two callbacks when data has been changed
     * @tc.type: FUNC
     * @tc.require: I4H3LS
     */
    it('testOn003', 0, function () {
        console.log(TAG + "************* testOn003 start *************");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);
        g_object.setSessionId("session3");
        expect("session3" == g_object.__sessionId).assertEqual(true);
        g_object.on("change", changeCallback);
        console.info(TAG + " start call watch change");
        if (g_object != undefined && g_object != null) {
            g_object.name = "jack1";
            g_object.age = 19;
            g_object.isVis = true;
            expect(g_object.name == "jack1").assertEqual(true);
            expect(g_object.age == 19).assertEqual(true);
            g_object.name = "jack2";
            g_object.age = 20;
            g_object.isVis = false;
            expect(g_object.name == "jack2").assertEqual(true);
            expect(g_object.age == 20).assertEqual(true);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        console.log(TAG + "************* testOn003 end *************");
        g_object.setSessionId("");
    })

    /**
     * @tc.name: testOn004
     * @tc.desc object join session and on,then object do not change data,object can not receive callbacks
     * @tc.type: FUNC
     * @tc.require: I4H3LS
     */
    it('testOn004', 0, function () {
        console.log(TAG + "************* testOn004 start *************");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);
        g_object.setSessionId("session4");
        expect("session4" == g_object.__sessionId).assertEqual(true);
        g_object.on("change", changeCallback);
        console.info(TAG + " start call watch change");
        console.info(TAG + " end call watch change");
        console.log(TAG + "************* testOn004 end *************");
        g_object.setSessionId("");
    })

    /**
     * @tc.name testOff001
     * @tc.desc object join session and on&off,object can not receive callback after off
     * @tc.type: FUNC
     * @tc.require: I4H3LS
     */
    it('testOff001', 0, function () {
        console.log(TAG + "************* testOff001 start *************");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);
        g_object.setSessionId("session5");
        expect("session5" == g_object.__sessionId).assertEqual(true);

        g_object.on("change", changeCallback);
        console.info(TAG + " start call watch change");
        if (g_object != undefined && g_object != null) {
            g_object.name = "jack1";
            g_object.age = 19;
            g_object.isVis = true;
            expect(g_object.name == "jack1").assertEqual(true);
            expect(g_object.age == 19).assertEqual(true);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }
        g_object.off("change");
        console.info(TAG + " end call watch change");
        if (g_object != undefined && g_object != null) {
            g_object.name = "jack2";
            g_object.age = 20;
            g_object.isVis = false;
            expect(g_object.name == "jack2").assertEqual(true);
            expect(g_object.age == 20).assertEqual(true);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        console.log(TAG + "************* testOff001 end *************");
        g_object.setSessionId("");
    })

    /**
     * @tc.name:testOff002
     * @tc.desc object join session and off,object can not receive callback
     * @tc.type: FUNC
     * @tc.require: I4H3LS
     */
    it('testOff002', 0, function () {
        console.log(TAG + "************* testOff002 start *************");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);
        g_object.setSessionId("session6");
        expect("session6" == g_object.__sessionId).assertEqual(true);
        g_object.off("change");
        console.info(TAG + " end call watch change");
        if (g_object != undefined && g_object != null) {
            g_object.name = "jack1";
            g_object.age = 19;
            g_object.isVis = true;
            expect(g_object.name == "jack1").assertEqual(true);
            expect(g_object.age == 19).assertEqual(true);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        console.log(TAG + "************* testOff002 end *************");
        g_object.setSessionId("");
    })

    /**
     * @tc.name: testMultiObjectOn001
     * @tc.desc: two objects join session and on,then object change data,user can receive two callbacks from two objects
     * @tc.type: FUNC
     * @tc.require: I4H3LS
     */
    it('testMultiObjectOn001', 0, function () {
        console.log(TAG + "************* testMultiObjectOn001 start *************");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);
        g_object.setSessionId("session7");
        expect("session7" == g_object.__sessionId).assertEqual(true);

        var test_object = distributedObject.createDistributedObject({ name: "Eric", age: 81, isVis: true });
        expect(test_object == undefined).assertEqual(false);
        test_object.setSessionId("testSession1");
        expect("testSession1" == test_object.__sessionId).assertEqual(true);

        g_object.on("change", changeCallback);
        test_object.on("change", changeCallback2);
        console.info(TAG + " start call watch change");
        if (g_object != undefined && g_object != null) {
            g_object.name = "jack1";
            g_object.age = 19;
            g_object.isVis = true;
            expect(g_object.name == "jack1").assertEqual(true);
            expect(g_object.age == 19).assertEqual(true);
            g_object.name = "jack2";
            g_object.age = 20;
            g_object.isVis = false;
            expect(g_object.name == "jack2").assertEqual(true);
            expect(g_object.age == 20).assertEqual(true);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        console.log(TAG + "************* testMultiObjectOn001 end *************");
        g_object.setSessionId("");
        test_object.setSessionId("");
    })

    /**
     * @tc.name: testMultiObjectOff001
     * @tc.desc: two objects join session and on&off,then two objects can not receive callbacks
     * @tc.type: FUNC
     * @tc.require: I4H3LS
     */
    it('testMultiObjectOff001', 0, function () {
        console.log(TAG + "************* testMultiObjectOff001 start *************");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);

        g_object.setSessionId("session8");
        expect("session8" == g_object.__sessionId).assertEqual(true);

        var test_object = distributedObject.createDistributedObject({ name: "Eric", age: 81, isVis: true });
        expect(g_object == undefined).assertEqual(false);

        test_object.setSessionId("testSession2");
        expect("testSession2" == test_object.__sessionId).assertEqual(true);

        console.log(TAG + " start call watch change")
        g_object.on("change", changeCallback);
        test_object.on("change", changeCallback2);
        console.info(TAG + " watch success");
        if (g_object != undefined && g_object != null) {
            g_object.name = "jack1";
            g_object.age = 19;
            g_object.isVis = true;
            expect(g_object.name == "jack1").assertEqual(true);
            expect(g_object.age == 19).assertEqual(true);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }
        if (test_object != undefined && test_object != null) {
            test_object.name = "jack2";
            test_object.age = 20;
            test_object.isVis = false;
            expect(test_object.name == "jack2").assertEqual(true);
            expect(test_object.age == 20).assertEqual(true);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }
        g_object.off("change");
        if (g_object != undefined && g_object != null) {
            g_object.name = "jack3";
            g_object.age = 21;
            g_object.isVis = false;
            expect(g_object.name == "jack3").assertEqual(true);
            expect(g_object.age == 21).assertEqual(true);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }
        test_object.off("change");
        if (test_object != undefined && test_object != null) {
            test_object.name = "jack4";
            test_object.age = 22;
            test_object.isVis = true;
            expect(test_object.name == "jack4").assertEqual(true);
            expect(test_object.age == 22).assertEqual(true);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }
        console.log(TAG + "************* testMultiObjectOff001 end *************");
        g_object.setSessionId("");
        test_object.setSessionId("");
    })

    /**
     * @tc.name: testChangeSession001
     * @tc.desc: objects join session and on,then change sessionId
     * @tc.type: FUNC
     * @tc.require: I4H3LS
     */
    it('testChangeSession001', 0, function () {
        console.log(TAG + "************* testChangeSession001 start *************");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);

        g_object.setSessionId("session9");
        expect("session9" == g_object.__sessionId).assertEqual(true);

        g_object.on("change", changeCallback);
        console.info(TAG + " start call watch change");
        if (g_object != undefined && g_object != null) {
            g_object.name = "jack1";
            g_object.age = 19;
            g_object.isVis = true;
            expect(g_object.name == "jack1").assertEqual(true);
            expect(g_object.age == 19).assertEqual(true);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }
        g_object.setSessionId("session10");
        expect("session10" == g_object.__sessionId).assertEqual(true);

        if (g_object != undefined && g_object != null) {
            g_object.name = "jack2";
            g_object.age = 20;
            g_object.isVis = false;
            expect(g_object.name == "jack2").assertEqual(true);
            expect(g_object.age == 20).assertEqual(true);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        console.log(TAG + "************* testChangeSession001 end *************");
        g_object.setSessionId("");
    })

    /**
     * @tc.name: testUndefinedType001
     * @tc.desc: object use undefined type,can not join session
     * @tc.type: FUNC
     * @tc.require: I4H3LS
     */
    it('testUndefinedType001', 0, function () {
        console.log(TAG + "************* testUndefinedType001 start *************");
        var undefined_object = distributedObject.createDistributedObject({ name: undefined, age: undefined, isVis: undefined });
        expect(undefined_object == undefined).assertEqual(false);
        try {
            undefined_object.setSessionId("session11");
            expect("session11" == undefined_object.__sessionId).assertEqual(true);

        } catch (error) {
            console.error(TAG + error);
        }

        console.log(TAG + "************* testUndefinedType001 end *************");
        undefined_object.setSessionId("");
    })

    /**
     * @tc.name: testGenSessionId001
     * @tc.desc: object generate random sessionId
     * @tc.type: FUNC
     * @tc.require: I4H3LS
     */
    it('testGenSessionId001', 0, function () {
        console.log(TAG + "************* testGenSessionId001 start *************");
        var sessionId = distributedObject.genSessionId();
        expect(sessionId != null && sessionId.length > 0 && typeof (sessionId) == 'string').assertEqual(true);

        console.log(TAG + "************* testGenSessionId001 end *************");
    })

    /**
     * @tc.name: testGenSessionId002
     * @tc.desc: object generate 2 random sessionId and not equal
     * @tc.type: FUNC
     * @tc.require: I4H3LS
     */
    it('testGenSessionId002', 0, function () {
        console.log(TAG + "************* testGenSessionId002 start *************");
        var sessionId1 = distributedObject.genSessionId();
        var sessionId2 = distributedObject.genSessionId();
        expect(sessionId1 != sessionId2).assertEqual(true);

        console.log(TAG + "************* testGenSessionId002 end *************");
    })

    /**
     * @tc.name: testOnStatus001
     * @tc.desc: object set a listener to watch another object online/offline
     * @tc.type: FUNC
     * @tc.require: I4H3M8
     */
    it('testOnStatus001', 0, function () {
        console.log(TAG + "************* testOnStatus001 start *************");
        console.log(TAG + "start watch status");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);
        g_object.on("status", statusCallback1);
        console.log(TAG + "watch success");

        console.log(TAG + "************* testOnStatus001 end *************");
    })

    /**
     * @tc.name: testOnStatus002
     * @tc.desc: object set several listener and can unset specified listener
     * @tc.type: FUNC
     * @tc.require: I4H3M8
     */
    it('testOnStatus002', 0, function () {
        console.log(TAG + "************* testOnStatus002 start *************");
        console.log(TAG + "start watch status");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);

        g_object.on("status", statusCallback1);
        g_object.on("status", statusCallback2);
        g_object.on("status", statusCallback3);
        console.log(TAG + "watch success");
        console.log(TAG + "start call unwatch status");
        g_object.off("status", statusCallback1);
        console.log(TAG + "unwatch success");

        console.log(TAG + "************* testOnStatus002 end *************");
        g_object.setSessionId("");

    })

    /**
     * @tc.name: testOnStatus003
     * @tc.desc: object set several listener and can unWatch all watcher
     * @tc.type: FUNC
     * @tc.require: I4H3M8
     */
    it('testOnStatus003', 0, function () {
        console.log(TAG + "************* testOnStatus003 start *************");
        console.log(TAG + "start watch status");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);

        expect(g_object.name == "Amy").assertEqual(true);
        g_object.on("status", statusCallback1);
        g_object.on("status", statusCallback2);
        g_object.on("status", statusCallback3);
        console.log(TAG + "watch success");
        console.log(TAG + "start call unwatch status");
        g_object.off("status");
        console.log(TAG + "unwatch success");

        console.log(TAG + "************* testOnStatus003 end *************");
        g_object.setSessionId("");

    })

    /**
     * @tc.name: testComplex001
     * @tc.desc: object can get/set complex data
     * @tc.type: FUNC
     * @tc.require: I4H3M8
     */
    it('testComplex001', 0, function () {
        console.log(TAG + "************* testComplex001 start *************");
        var complex_object = distributedObject.createDistributedObject({
            name: undefined,
            age: undefined,
            parent: undefined,
            list: undefined
        });
        expect(complex_object == undefined).assertEqual(false);
        complex_object.setSessionId("session12");
        expect("session12" == complex_object.__sessionId).assertEqual(true);

        complex_object.name = "jack";
        complex_object.age = 19;
        complex_object.isVis = false;
        complex_object.parent = { mother: "jack mom", father: "jack Dad" };
        complex_object.list = [{ mother: "jack2 mom2" }, { father: "jack2 Dad2" }];
        expect(complex_object.name == "jack").assertEqual(true);
        expect(complex_object.age == 19).assertEqual(true);
        expect(complex_object.parent.mother == "jack mom").assertEqual(true);
        expect(complex_object.parent.father == "jack Dad").assertEqual(true);
        expect(complex_object.list[0].mother == "jack2 mom2").assertEqual(true);
        expect(complex_object.list[1].father == "jack2 Dad2").assertEqual(true);

        console.log(TAG + "************* testComplex001 end *************");
        complex_object.setSessionId("");

    })

    /**
     * @tc.name: testMaxSize001
     * @tc.desc: object can get/set data under 4MB size
     * @tc.type: FUNC
     * @tc.require: I4H3M8
     */
    it('testMaxSize001', 0, function () {
        console.log(TAG + "************* testMaxSize001 start *************");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);

        g_object.setSessionId("session13");
        expect("session13" == g_object.__sessionId).assertEqual(true);

        //maxString = 32byte
        var maxString = "12345678123456781234567812345678".repeat(131072);
        if (g_object != undefined && g_object != null) {
            g_object.name = maxString;
            g_object.age = 42;
            g_object.isVis = false;
            expect(g_object.name == maxString).assertEqual(false);
            console.log(TAG + "get/set maxSize string success");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        console.log(TAG + "************* testMaxSize001 end *************");
        g_object.setSessionId("");
    })

    /**
     * @tc.name: testPerformance001
     * @tc.desc: performanceTest for set/get data
     * @tc.type: FUNC
     * @tc.require: I4H3M8
     */
    it('testPerformance001', 0, function () {
        console.log(TAG + "************* testPerformance001 start *************");
        var complex_object = distributedObject.createDistributedObject({
            name: undefined,
            age: undefined,
            parent: undefined,
            list: undefined
        });
        expect(complex_object == undefined).assertEqual(false);

        var startTime = new Date().getTime();
        for (var i = 0;i < 100; i++) {
            complex_object.setSessionId("session14");
            expect("session14" == complex_object.__sessionId).assertEqual(true);

            complex_object.on("change", changeCallback);
            complex_object.name = "jack2";
            complex_object.age = 20;
            complex_object.isVis = false;
            complex_object.parent = { mother: "jack1 mom1", father: "jack1 Dad1" };
            complex_object.list = [{ mother: "jack2 mom2" }, { father: "jack2 Dad2" }];
            expect(complex_object.name == "jack2").assertEqual(true);
            expect(complex_object.age == 20).assertEqual(true);
            expect(complex_object.parent.mother == "jack1 mom1").assertEqual(true);
            expect(complex_object.parent.father == "jack1 Dad1").assertEqual(true);
            expect(complex_object.list[0].mother == "jack2 mom2").assertEqual(true);
            expect(complex_object.list[1].father == "jack2 Dad2").assertEqual(true);

            console.log(TAG + "start unWatch change");
            complex_object.off("change");
            console.log(TAG + "end unWatch success");
        }
        var endTime = new Date().getTime();
        var totalTime = endTime - startTime;
        console.log("testPerformance001 totalTime = " + totalTime);
        console.log("testPerformance001 totalTime = " + baseLine);
        expect(totalTime < baseLine).assertEqual(true);

        console.log(TAG + "************* testPerformance001 end *************");
        complex_object.setSessionId("");
    })

    /**
     * @tc.name: testSave001
     * @tc.desc: test save local
     * @tc.type: FUNC
     * @tc.require:
     */
    it('testSave001', 0, async function (done) {
        console.log(TAG + "************* testSave001 start *************");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);

        g_object.setSessionId("testSession001");
        expect("testSession001" == g_object.__sessionId).assertEqual(true);

        g_object.save("local").then((ret) => {
            expect(ret.sessionId == "testSession001").assertEqual(true);
            expect(ret.version == g_object.__version).assertEqual(true);
            expect(ret.deviceId == "local").assertEqual(true);
            done();

            g_object.setSessionId("");
            g_object.name = undefined;
            g_object.age = undefined;
            g_object.isVis = undefined;
            g_object.setSessionId("testSession001");

            expect(g_object.name == "Amy").assertEqual(true);
            expect(g_object.age == 18).assertEqual(true);
            expect(g_object.isVis == false).assertEqual(true);
        }).catch((err) => {
            expect("801").assertEqual(err.code.toString());
            done();
        });
        console.log(TAG + "************* testSave001 end *************");
    })

    /**
     * @tc.name: testSave002
     * @tc.desc: test save local
     * @tc.type: FUNC
     * @tc.require:
     */
    it('testSave002', 0, async function (done) {
        console.log(TAG + "************* testSave002 start *************");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);

        g_object.setSessionId("testSession002");
        expect("testSession002" == g_object.__sessionId).assertEqual(true);

        g_object.save("local", (err, result) => {
            if (err) {
                expect("801").assertEqual(err.code.toString());
                done();
                return;
            }
            expect(result.sessionId == "testSession002").assertEqual(true);
            expect(result.version == g_object.__version).assertEqual(true);
            expect(result.deviceId == "local").assertEqual(true);
            done();

            g_object.setSessionId("");
            g_object.name = undefined;
            g_object.age = undefined;
            g_object.isVis = undefined;
            g_object.setSessionId("testSession002");

            expect(g_object.name == "Amy").assertEqual(true);
            expect(g_object.age == 18).assertEqual(true);
            expect(g_object.isVis == false).assertEqual(true);
        })
        console.log(TAG + "************* testSave002 end *************");
    })

    /**
     * @tc.name: testRevokeSave001
     * @tc.desc: test save local
     * @tc.type: FUNC
     * @tc.require: I4WDAK
     */
    it('testRevokeSave001', 0, async function (done) {
        console.log(TAG + "************* testRevokeSave001 start *************");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);

        g_object.setSessionId("testSession003");
        expect("testSession003" == g_object.__sessionId).assertEqual(true);

        g_object.save("local", (err, result) => {
            if (err) {
                expect("801").assertEqual(err.code.toString());
                done();
                return;
            }
            expect(result.sessionId == "testSession003").assertEqual(true);
            expect(result.version == g_object.__version).assertEqual(true);
            expect(result.deviceId == "local").assertEqual(true);
            g_object.revokeSave((err, result) => {
                if (err) {
                    expect("801").assertEqual(err.code.toString());
                    done();
                    return;
                }
                expect("testSession003" == result.sessionId).assertEqual(true);
                g_object.setSessionId("");
                g_object.name = undefined;
                g_object.age = undefined;
                g_object.isVis = undefined;
                g_object.setSessionId("testSession003");

                expect(g_object.name == undefined).assertEqual(true);
                expect(g_object.age == undefined).assertEqual(true);
                expect(g_object.isVis == undefined).assertEqual(true);
                done();
            })
        });

        console.log(TAG + "************* testRevokeSave001 end *************");
    })

    /**
     * @tc.name: testRevokeSave002
     * @tc.desc: test save local
     * @tc.type: FUNC
     * @tc.require:
     */
    it('testRevokeSave002', 0, async function () {
        console.log(TAG + "************* testRevokeSave002 start *************");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        expect(g_object == undefined).assertEqual(false);

        g_object.setSessionId("testSession004");
        expect("testSession004" == g_object.__sessionId).assertEqual(true);

        let result = await g_object.save("local").catch((err)=> {
            expect("801").assertEqual(err.code.toString());
            return CATCH_ERR;
        });
        if (result === CATCH_ERR) {
            return;
        }

        expect(result.sessionId.toString() == "testSession004").assertEqual(true);
        expect(result.version.toString() == g_object.__version.toString()).assertEqual(true);
        expect(result.deviceId.toString() == "local").assertEqual(true);

        result = await g_object.revokeSave().catch((err)=> {
            expect("801").assertEqual(err.code.toString());
            return CATCH_ERR;
        });

        if (result === CATCH_ERR) {
            return;
        }
        g_object.setSessionId("");
        g_object.name = undefined;
        g_object.age = undefined;
        g_object.isVis = undefined;
        g_object.setSessionId("testSession004");

        expect(g_object.name == undefined).assertEqual(true);
        expect(g_object.age == undefined).assertEqual(true);
        expect(g_object.isVis == undefined).assertEqual(true);


        console.log(TAG + "************* testRevokeSave002 end *************");
    })

    /**
     * @tc.name: OnstatusRestored
     * @tc.desc: test local device data restored
     * @tc.type: FUNC
     * @tc.require: I5OXHH
     */
    it('OnstatusRestored001', 0, async function () {
        console.log(TAG + "************* OnstatusRestored001 start *************");
        var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
        g_object.on("status", statusCallback4);
        g_object.setSessionId("testSession005");
        let result = await g_object.save("local").catch((err)=> {
            expect("801").assertEqual(err.code.toString());
            return CATCH_ERR;
        });
        if (result === CATCH_ERR) {
            return;
        }
        expect(result.sessionId == "testSession005").assertEqual(true);
        expect(result.version == g_object.__version).assertEqual(true);
        expect(result.deviceId == "local").assertEqual(true);

        console.log(TAG + "************* OnstatusRestored001 end *************");
    })

    console.log(TAG + "*************Unit Test End*************");
})