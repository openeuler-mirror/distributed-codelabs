import prompt from '@ohos.prompt';
import hilog from '@ohos.hilog';
import softbus_client_run from 'libsoftbus_client_run.so'
var thisOld

@Entry
@Component
struct Index {
  @State sessionServerInitState: string = "null"
  @State sessionConnectState: string = "null"
  @State sendDataState: string = ''
  @State receiveData: string = ''
  @State hasNewData: boolean = false
  @State isLoading: boolean = false
  @State deviceInfoArray: Array<any> = []
  @State deviceIndex: number = undefined

  callbackPictureSave(data) {
    hilog.info(0, "SoftBusClient", "[INFO] Data Received: %s", data)
    thisOld.hasNewData = true
    thisOld.receiveData = data
  }

  build() {
    Stack() {
      Column() {
        Row() {
          if (thisOld.hasNewData) {
            Text(thisOld.receiveData)
              .fontSize(16)
              .fontColor($r('app.color.white'))
          }
        }
        .justifyContent(FlexAlign.Center)
        .width('100%')
        .height('8%')
        .padding(10)
        .backgroundColor($r('app.color.transparent_grey'))

        Row() {
          Button({ type: ButtonType.Circle, stateEffect: true }) {
            if (this.isLoading) {
              LoadingProgress()
                .color(Color.White)
                .width(70)
                .height(70)
            } else {
              Image($r('app.media.refresh'))
                .width(48)
                .height(48)
                .objectFit(ImageFit.Fill)
            }
          }
          .width(64).height(64)
          .backgroundColor($r('app.color.transparent'))
          .onClick(() => {
            this.isLoading = true
            this.deviceInfoArray.length = 0
            this.deviceIndex = undefined
            this.sessionConnectState = softbus_client_run.sessionConnect()
            this.isLoading = false
            this.parseDeviceInfo(this.sessionConnectState)
          })
        }
        .width('90%')
        .height('8%')
        .justifyContent(FlexAlign.Start)

        Column() {
          Text('终端：' + this.deviceInfoArray.length.toString())
            .fontColor($r('app.color.white'))
            .fontSize(16)
          List() {
            ForEach(this.deviceInfoArray, (items) => {
              ListItem() {
                Row() {
                  Column() {
                    Image($r('app.media.phone'))
                      .width(42)
                      .height(42)
                      .objectFit(ImageFit.Cover)
                      .onClick(() => {
                        this.deviceIndex = this.deviceInfoArray.indexOf(items)
                      })
                    Text(items[0]).maxLines(2).fontColor($r('app.color.white'))
                      .fontSize(16)
                      .textAlign(TextAlign.Center)
                      .margin({ left: 12 })
                  }

                  if (this.deviceIndex == this.deviceInfoArray.indexOf(items)) {
                    Text(items[1])
                      .fontColor($r('app.color.white'))
                      .fontSize(20)
                      .textAlign(TextAlign.Start)
                      .margin({ left: 12 })
                  }
                }
                .width('100%')
                .margin(8)
              }
            })
          }.padding(2).height('82%')
        }
        .alignItems(HorizontalAlign.Start)
        .margin({ top: 20 })
        .width('72%')
        .height('48%')

        Row() {
          Column() {
            Button($r('app.string.Unlock'), { type: ButtonType.Normal })
              .borderRadius(8)
              .fontSize(24)
              .height('18%')
              .width('60%')
              .backgroundColor($r('app.color.button_color'))
              .margin(5)
              .onClick(() => {
                let sendResult = softbus_client_run.sendData("session_test", "Hello openEuler", 15)
                if (sendResult == 0) {
                  this.sendDataState = "Send Successful"
                } else {
                  this.sendDataState = "Send Failed"
                }
                prompt.showToast({
                  message: this.sendDataState,
                  bottom: '20px'
                })
              })
          }
          .justifyContent(FlexAlign.SpaceEvenly)
          .width('50%')
          .height('100%')
        }
        .height('34%')
        .width('100%')
        .justifyContent(FlexAlign.SpaceEvenly)
        .padding({ bottom: 8 })
        .backgroundColor('#07309E')
      }
      .width('100%')
      .height('100%')
      .backgroundImage($r('app.media.background'))
      .backgroundImageSize({ width: '100%', height: '75%' })
    }
  }

  aboutToAppear() {
    //进入页面初始化
    thisOld = this
    this.sessionServerInitState = softbus_client_run.sessionServerInit(this.callbackPictureSave)
    prompt.showToast({
      message: this.sessionServerInitState,
      bottom: '20px'
    })
  }

  aboutToDisappear() {
    //关闭
    softbus_client_run.sessionDisconnect()
  }

  parseDeviceInfo(deviceInfo: string) {
    let deviceInfoArrayTemp = deviceInfo.split(';')
    deviceInfoArrayTemp.pop()
    deviceInfoArrayTemp.forEach(element => {
      let startIndex = element.indexOf(': ')
      let endIndex = element.indexOf('\nipAddress')
      let deviceName = element.substring(startIndex + 1, endIndex)
      this.deviceInfoArray.push([deviceName, element])
    });
  }
}