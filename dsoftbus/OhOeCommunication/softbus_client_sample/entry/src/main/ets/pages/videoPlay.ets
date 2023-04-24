import router from '@ohos.router';
import hilog from '@ohos.hilog';


@Entry
@Component
export default struct VideoPlay {
  @State imageBgColor: ResourceStr = $r('app.color.transparent')
  @State muted: boolean = false
  @State autoPlay: boolean = true
  @State controls: boolean = true //控制栏
  myVideoController: VideoController = new VideoController()
  @State pause: boolean = false

  build() {
    //操作
    Column() {
      Row() {
        Image($r('app.media.back'))
          .width(32)
          .height(32)
          .borderRadius(32)
          .objectFit(ImageFit.Cover)
          .margin({ left: 8 })
          .backgroundColor(this.imageBgColor)
          .onClick(() => {
            this.imageBgColor = $r('app.color.transparent_grey')
            router.back()
          })
      }
      .width('100%')
      .height('6%')
      .backgroundColor($r('app.color.transparent_grey'))
      .justifyContent(FlexAlign.Start)

      Video({
        src: $rawfile('fire_detection.mp4'), // 视频播放源的路径
        controller: this.myVideoController // 控制器
      })
        .muted(this.muted) // 是否静音
        .autoPlay(this.autoPlay) // 是否自动播放
        .controls(this.controls) // 控制视频播放的控制栏是否显示
        .objectFit(ImageFit.Contain) // 视频显示模式
        .loop(false) // 是否单个视频循环播放
        .width('100%')
        .aspectRatio(1.33)
        .onClick(() => {
          //点击视频调出控制栏
          this.controls = !this.controls
        })
        .onStart(() => {
          // 播放时触发该事件
          hilog.info(0, "SoftBusClient", "[INFO] the video start play")
        })
        .onPause(() => {
          // 暂停时触发该事件
          hilog.info(0, "SoftBusClient", "[INFO] [INFO] the video pause play")
        })
        .onFinish(() => {
          // 播放结束时触发该事件
        })

      Row() {
        Button({ type: ButtonType.Circle }) {
          Image(this.pause ? $r('app.media.play_circle') : $r('app.media.pause_circle'))
        }
        .height('60%')
        .backgroundColor($r('app.color.transparent_grey'))
        .onClick(() => {
          this.pause = !this.pause
          this.pause ? this.myVideoController.pause() : this.myVideoController.start()
        })

        Button({ type: ButtonType.Circle }) {
          Image($r('app.media.full_screen')).rotate({
            angle: -45
          })
        }
        .height('60%')
        .backgroundColor($r('app.color.transparent_grey'))
        .onClick(() => {
          this.myVideoController.requestFullscreen(true)
        })
      }
      .justifyContent(FlexAlign.SpaceBetween)
      .width('100%')
      .height('8%')
      .backgroundColor($r('app.color.transparent_grey'))
    }
    .width('100%')
    .height('100%')
  }

  aboutToAppear() {
    //进入之后就全屏播放
    this.myVideoController.start()
  }

  aboutToDisappear() {
    router.clear();
  }
}