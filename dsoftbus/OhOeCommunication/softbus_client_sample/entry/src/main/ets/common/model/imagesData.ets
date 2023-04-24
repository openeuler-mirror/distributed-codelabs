import fileio from '@ohos.fileio'
import hilog from '@ohos.hilog';


export default class ImageData {
  getAIImageData(callback) {
    //刚进入页面获取文件夹内容
    let dirPath = '/data/storage/el2/base/haps/entry/files/ai_analysis/'
    let imageFiles = []
    hilog.info(0, "SoftBusClient", "[INFO] Start to get images form: %s", dirPath)
    this.readImages(dirPath, imageFiles)

    hilog.info(0, "SoftBusClient", "[INFO] New imageFiles length: %d", imageFiles.length)
    // return imageFiles
    callback(imageFiles)
  }

  getSituationImageData(callback) {
    //刚进入页面获取文件夹内容
    let dirPath = '/data/storage/el2/base/haps/entry/files/situation_awareness/'
    let imageFiles = []
    hilog.info(0, "SoftBusClient", "[INFO] Start to get images form: %s", dirPath)
    this.readImages(dirPath, imageFiles)

    hilog.info(0, "SoftBusClient", "[INFO] New imageFiles length: %d", imageFiles.length)
    // return imageFiles
    callback(imageFiles)
  }

  readImages(dirPath, imageFiles) {
    let dir = fileio.opendirSync(dirPath); // dir 管理目录
    hilog.debug(0, "SoftBusClient", "[INFO] Read dir: %s", JSON.stringify(dir))
    this.inquireFiles(dirPath, dir, imageFiles);
  }

  inquireFiles(dirPath, dir, imageFiles) {
    let dirent = dir.readSync(); // 读取下一个目录项
    if (dirent && dirent.isFile()) {
      let formatImageFile = "file://" + dirPath + dirent.name
      hilog.debug(0, "SoftBusClient", "[INFO] Get images file: %s", formatImageFile)
      imageFiles.push(formatImageFile)
      this.inquireFiles(dirPath, dir, imageFiles); // 循环获取下一个目录
    } else {
      dir.closeSync(); // 关闭目录，释放dir中的文件描述
    }
  }
}
