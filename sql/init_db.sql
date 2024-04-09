DROP TABLE IF EXISTS `highscores`;
CREATE TABLE `highscores` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(30) NOT NULL,
  `skin` varchar(20) NOT NULL,
  `map_id` int(11) NOT NULL,
  `time` int(11) DEFAULT NULL,
  `time_string` text DEFAULT NULL,
  `datetime` datetime DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

